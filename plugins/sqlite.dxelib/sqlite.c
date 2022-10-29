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

    int rc = sqlite3_open(fname, &sql->db);
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

    sqlite_t *sql = js_touserdata(J, 0, TAG_SQLITE);
    if (sql->db) {
        const char *stmt = js_tostring(J, 1);

        res.J = J;
        res.count = 0;
        js_newarray(J);
        int rc = sqlite3_exec(sql->db, stmt, SQLite_callback, &res, &zErrMsg);
        if (rc != SQLITE_OK) {
            js_pop(J, 1);
            js_error(J, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
            return;
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
