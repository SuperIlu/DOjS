/*
MIT License

Copyright (c) 2019-2021 Andre Seidelt <superilu@yahoo.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <mujs.h>
#include <stdio.h>
#include <stdlib.h>

#include "DOjS.h"

#include "bytearray.h"

#include "sqlite.h"
#include "sqlite3.h"

/************
** structs **
************/
//! userdata definition
typedef struct __sqlite {
    sqlite3 *db;  //!< the database
} sqlite_t;

//! user data strucvture for sqlite3_exec()
typedef struct __result {
    js_State *J;         //!< VM state
    unsigned int count;  //!< current count of result lines
} result_t;

/*********************
** static functions **
*********************/
/**
 * @brief sqlite3_exec() callback function
 *
 * @param user userdata, a result_t struct.
 * @param argc number of columns
 * @param argv columns data
 * @param azColName column names
 *
 * @return int 0 for no error, else !=0.
 */
static int SQLite_callback(void *user, int argc, char **argv, char **azColName) {
    result_t *res = (result_t *)user;

    js_newobject(res->J);
    for (int i = 0; i < argc; i++) {
        DEBUGF("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        if (argv[i]) {
            js_pushstring(res->J, argv[i]);
        } else {
            js_pushnull(res->J);
        }
        js_setproperty(res->J, -2, azColName[i]);
    }
    js_setindex(res->J, -2, res->count);
    res->count++;
    return 0;
}

/**
 * @brief finalize a database and free resources.
 *
 * @param J VM state.
 */
static void SQLite_Finalize(js_State *J, void *data) {
    sqlite_t *sql = (sqlite_t *)data;
    if (sql->db) {
        sqlite3_close(sql->db);
        sql->db = NULL;
    }
    free(sql);
}

/**
 * @brief open a database
 * sql = new SQLite(filename:string)
 *
 * @param J VM state.
 */
static void new_SQLite(js_State *J) {
    NEW_OBJECT_PREP(J);
    const char *fname = js_tostring(J, 1);

    sqlite_t *sql = calloc(1, sizeof(sqlite_t));
    if (!sql) {
        JS_ENOMEM(J);
        return;
    }

    int rc = sqlite3_open_v2(fname, &sql->db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, "unix-none");
    if (rc) {
        js_error(J, "Can't open database: %s\n", sqlite3_errmsg(sql->db));
        sqlite3_close(sql->db);
        free(sql);
        return;
    }

    js_currentfunction(J);
    js_getproperty(J, -1, "prototype");
    js_newuserdata(J, TAG_SQLITE, sql, SQLite_Finalize);
}

/**
 * @brief close the database.
 * sql.Close()
 *
 * @param J VM state.
 */
static void SQLite_Close(js_State *J) {
    sqlite_t *sql = js_touserdata(J, 0, TAG_SQLITE);
    if (sql->db) {
        sqlite3_close(sql->db);
        sql->db = NULL;
    }
}

/**
 * @brief execute a SQL statement on the db.
 * sql.Exec("SELECT * FROM test;")
 *
 * @param J VM state.
 */
static void SQLite_Exec(js_State *J) {
    char *zErrMsg = NULL;
    result_t res;
    int rc;

    sqlite_t *sql = js_touserdata(J, 0, TAG_SQLITE);
    if (sql->db) {
        if (js_isstring(J, 1) && js_isarray(J, 2)) {
            // LOGF("Prepared statement\n");
            // statement with parameters
            const char *stmt = js_tostring(J, 1);

            // prepare the statement
            sqlite3_stmt *prep_stmt;
            rc = sqlite3_prepare_v2(sql->db, stmt, -1, &prep_stmt, NULL);
            if (SQLITE_OK != rc) {
                js_error(J, "Can't prepare statment %s (%i): %s\n", stmt, rc, sqlite3_errmsg(sql->db));
                return;
            }

            // add all parameters
            int len = js_getlength(J, 2);
            for (int i = 0; i < len; i++) {
                js_getindex(J, 2, i);
                if (js_isuserdata(J, -1, TAG_BYTE_ARRAY)) {
                    byte_array_t *ba = js_touserdata(J, -1, TAG_BYTE_ARRAY);
                    rc = sqlite3_bind_blob(prep_stmt, i + 1, ba->data, ba->size, SQLITE_STATIC);
                    // LOGF("Bound ByteArray %d\n", ba->size);
                } else if (js_isnumber(J, -1)) {
                    rc = sqlite3_bind_double(prep_stmt, i + 1, js_tonumber(J, -1));
                    // LOGF("Bound %f\n", js_tonumber(J, -1));
                } else if (js_isnull(J, -1)) {
                    rc = sqlite3_bind_null(prep_stmt, i + 1);
                    // LOGF("Bound NULL\n");
                } else if (js_isstring(J, -1)) {
                    rc = sqlite3_bind_text(prep_stmt, i + 1, js_tostring(J, -1), -1, SQLITE_STATIC);
                    // LOGF("Bound %s\n", js_tostring(J, -1));
                }
                js_pop(J, 1);

                if (SQLITE_OK != rc) {
                    js_error(J, "Can't bind parameter %i (%i): %s\n", i + 1, rc, sqlite3_errmsg(sql->db));
                    sqlite3_finalize(prep_stmt);
                    return;
                }
            }

            // execute statement and get results
            js_newarray(J);
            int count = 0;
            while (true) {
                rc = sqlite3_step(prep_stmt);
                if (rc == SQLITE_DONE) {
                    // query is done
                    break;
                } else if (rc == SQLITE_ROW) {
                    size_t blen;
                    const uint8_t *bdata;
                    // next row of data
                    int cols = sqlite3_column_count(prep_stmt);
                    if (cols > 0) {
                        js_newobject(J);
                        for (int i = 0; i < cols; i++) {
                            // LOGF("Column #%d %s is %d\n", i, sqlite3_column_name(prep_stmt, i), sqlite3_column_type(prep_stmt, i));
                            switch (sqlite3_column_type(prep_stmt, i)) {
                                case SQLITE_INTEGER:
                                case SQLITE_FLOAT:
                                    js_pushnumber(J, sqlite3_column_double(prep_stmt, i));
                                    // LOGF("Fetched number\n");
                                    break;
                                case SQLITE_TEXT:
                                    js_pushstring(J, (const char *)sqlite3_column_text(prep_stmt, i));
                                    // LOGF("Fetched string\n");
                                    break;
                                case SQLITE_NULL:
                                    js_pushnull(J);
                                    // LOGF("Fetched NULL\n");
                                    break;
                                case SQLITE_BLOB:
                                    blen = sqlite3_column_bytes(prep_stmt, i);
                                    bdata = sqlite3_column_blob(prep_stmt, i);
                                    ByteArray_fromBytes(J, bdata, blen);
                                    // LOGF("Fetched ByteArray\n");
                                    break;
                            }
                            js_setproperty(J, -2, sqlite3_column_name(prep_stmt, i));
                        }
                        js_setindex(J, -2, count);
                        count++;
                    }
                } else {
                    js_pop(J, 1);
                    js_error(J, "Can't execute statement (%i): %s\n", rc, sqlite3_errmsg(sql->db));
                    sqlite3_finalize(prep_stmt);
                    return;
                }
            }

            sqlite3_finalize(prep_stmt);
        } else {
            // LOGF("Regular statement\n");
            // we assume a plain statement
            const char *stmt = js_tostring(J, 1);

            res.J = J;
            res.count = 0;
            js_newarray(J);
            rc = sqlite3_exec(sql->db, stmt, SQLite_callback, &res, &zErrMsg);
            if (rc != SQLITE_OK) {
                js_pop(J, 1);
                js_error(J, "SQL error: %s\n", zErrMsg);
                sqlite3_free(zErrMsg);
                return;
            }
        }
    } else {
        js_error(J, "Database was closed");
    }
}

/**
 * @brief sqlite module init
 *
 * @param J VM state.
 */
void init_sqlite(js_State *J) {
    LOGF("%s\n", __PRETTY_FUNCTION__);

    js_newobject(J);
    {
        NPROTDEF(J, SQLite, Close, 0);
        NPROTDEF(J, SQLite, Exec, 1);
    }
    CTORDEF(J, new_SQLite, TAG_SQLITE, 1);
}
