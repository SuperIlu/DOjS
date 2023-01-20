/**
 * SQLite database.
 * 
 * **Note: SQLite module must be loaded by calling LoadLibrary("sqlite") before using!**
 * 
 * @class
 * @param {string} fname name of the database file. It will be created if it dows not exist.
 */
function SQLite(fname) { }
/**
 * Close the SQLite database.
 */
SQLite.prototype.Close = function () { };
/**
 * Execute a SQL query on the database. See SQLite documentation for query language details.
 * Note: you can use ByteArray to pass/retrieve BLOBs. These can be very memory consuming as all data is kept im RAM.
 * 
 * @param {string} sql the sql statement
 * @param {Object[]} [params] bind parameters that will be used if the sql had unbound parameters
 * 
 * @returns {Object[]} returns a list of objects for SQL queries. Each object has the column names as keys and the column contents as values.
 * 
 * @see https://www.sqlite.org/lang.html
 * @example
var sql = new SQLite("test.db");
//////
// create test table
exec(sql, "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT);");

// insert two lines
exec(sql, "INSERT INTO test (id, name) VALUES (42, 'zweiundvierzig');");
exec(sql, "INSERT INTO test (id, name) VALUES (23, 'dreiundzwanzig');");

// try duplicate check
exec(sql, "INSERT INTO test (id, name) VALUES (23, 'dreiundzwanzig');");
// Error: SQL error: UNIQUE constraint failed: test.id
//   at SQLite.prototype.Exec (native)

// select some data
var res1 = exec(sql, "SELECT * from test;");             // [{"id":"23","name":"dreiundzwanzig"},{"id":"42","name":"zweiundvierzig"}]
var res2 = exec(sql, "SELECT * from test WHERE id=23;"); // [{"id":"23","name":"dreiundzwanzig"}]

*/
SQLite.prototype.Exec = function (sql, params) { };
