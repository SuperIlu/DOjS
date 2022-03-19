LoadLibrary("sqlite");
Include("console");

function msg(s, c) {
	con.Log(s, c);
	Println(s);
}

function dump(s, c) {
	msg(JSON.stringify(s), c);
}

function exec(sql, stmt) {
	try {
		msg(stmt);
		var res = sql.Exec(stmt);
		if (res.length > 0) {
			dump(res, EGA.RED);
		}
	} catch (e) {
		msg(e);
	}
}

var con;

function commandFunc(c) {
	var sql = new SQLite("test.db");

	var inp = c.GetInput();
	if (inp.length > 0) {
		exec(sql, inp);

		c.SetInput("");
	}

	sql.Close();
}

function Setup() {
	con = new Console(EGA.GREEN, EGA.BLACK, commandFunc);

	try {
		RmFile("test.db");
	} catch (e) {
		msg(e);
	}

	var sql = new SQLite("test.db");
	//////
	// create test table
	exec(sql, "CREATE TABLE test (id INTEGER PRIMARY KEY, name TEXT);");

	// insert two lines
	exec(sql, "INSERT INTO test (id, name) VALUES (42, 'zweiundvierzig');");
	exec(sql, "INSERT INTO test (id, name) VALUES (23, 'dreiundzwanzig');");

	// try duplicate check
	exec(sql, "INSERT INTO test (id, name) VALUES (23, 'dreiundzwanzig');");

	// select some data
	exec(sql, "SELECT * from test;");
	exec(sql, "SELECT * from test WHERE id=23;");

	// drop table again
	//exec(sql, "DROP TABLE test;");


	//////
	// create, fill and drop a table
	exec(sql, "CREATE TABLE t1 (a PRIMARY KEY, b TEXT, c REAL);");

	exec(sql, "BEGIN TRANSACTION;");
	for (var i = 0; i < 10; i++) {
		exec(sql, "INSERT INTO t1 (a, b, c) VALUES (" + i + ", '" + i + "', " + (i / 3) + ");");
	}
	exec(sql, "COMMIT;");

	exec(sql, "CREATE INDEX idx_t1 ON t1 (b);");

	exec(sql, "UPDATE t1 set b='foo' where a=666;");
	exec(sql, "UPDATE t1 set b='bar' where a BETWEEN 23 AND 42;");

	exec(sql, "BEGIN TRANSACTION;");
	exec(sql, "DELETE FROM t1;");
	exec(sql, "ROLLBACK;");

	exec(sql, "SELECT * from t1 WHERE b='foo' OR b='bar';");

	//exec(sql, "DROP TABLE t1;");

	sql.Close();
}

function Loop() {
	con.Draw();
}

function Input(e) { con.HandleInput(e); }
