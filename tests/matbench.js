/*
** This function is called once when the script is started.
*/
var ITERATIONS = 2000;

var FUNCLIST = [
	"GetRotationMatrix",
	"GetTransformationMatrix",
	"GetXRotateMatrix",
	"GetYRotateMatrix",
	"GetZRotateMatrix",
	"MatrixMul",
	"ApplyMatrix",
	"PerspProject",
	"PolygonZNormal"
];

function Setup() {
	for (var i = 0; i < FUNCLIST.length; i++) {
		var cFunc = "test_" + FUNCLIST[i] + "();";
		var jsFunc = "test_N" + FUNCLIST[i] + "();";
		var cTime = eval(cFunc);
		var jsTime = eval(jsFunc);
		Println("Speedup is " + (jsTime / cTime) + "x\n");
	}
}

function test_NGetRotationMatrix() {
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		NGetRotationMatrix(i, i, i);
	}
	sw.Stop();
	sw.Print("NGetRotationMatrix");
	return sw.ResultMs();
}
function test_GetRotationMatrix() {
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		GetRotationMatrix(i, i, i);
	}
	sw.Stop();
	sw.Print("GetRotationMatrix");
	return sw.ResultMs();
}

function test_NGetTransformationMatrix() {
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		NGetTransformationMatrix(i, i, i, i, i, i, i);
	}
	sw.Stop();
	sw.Print("NGetTransformationMatrix");
	return sw.ResultMs();
}
function test_GetTransformationMatrix() {
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		GetTransformationMatrix(i, i, i, i, i, i, i);
	}
	sw.Stop();
	sw.Print("GetTransformationMatrix");
	return sw.ResultMs();
}

function test_NGetXRotateMatrix() {
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		NGetXRotateMatrix(i);
	}
	sw.Stop();
	sw.Print("NGetXRotateMatrix");
	return sw.ResultMs();
}
function test_NGetYRotateMatrix() {
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		NGetYRotateMatrix(i);
	}
	sw.Stop();
	sw.Print("NGetYRotateMatrix");
	return sw.ResultMs();
}
function test_NGetZRotateMatrix() {
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		NGetZRotateMatrix(i);
	}
	sw.Stop();
	sw.Print("NGetZRotateMatrix");
	return sw.ResultMs();
}
function test_GetXRotateMatrix() {
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		GetXRotateMatrix(i);
	}
	sw.Stop();
	sw.Print("GetXRotateMatrix");
	return sw.ResultMs();
}
function test_GetYRotateMatrix() {
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		GetYRotateMatrix(i);
	}
	sw.Stop();
	sw.Print("GetYRotateMatrix");
	return sw.ResultMs();
}
function test_GetZRotateMatrix() {
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		GetZRotateMatrix(i);
	}
	sw.Stop();
	sw.Print("GetZRotateMatrix");
	return sw.ResultMs();
}

function test_NMatrixMul() {
	var m1 = GetIdentityMatrix();
	var m2 = GetIdentityMatrix();

	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		NMatrixMul(m1, m2);
	}
	sw.Stop();
	sw.Print("NMatrixMul");
	return sw.ResultMs();
}
function test_MatrixMul() {
	var m1 = GetIdentityMatrix();
	var m2 = GetIdentityMatrix();

	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		MatrixMul(m1, m2);
	}
	sw.Stop();
	sw.Print("MatrixMul");
	return sw.ResultMs();
}

function test_NApplyMatrix() {
	var m1 = GetIdentityMatrix();

	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		NApplyMatrix(m1, i, i, i);
	}
	sw.Stop();
	sw.Print("NApplyMatrix");
	return sw.ResultMs();
}
function test_ApplyMatrix() {
	var m1 = GetIdentityMatrix();

	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		ApplyMatrix(m1, i, i, i);
	}
	sw.Stop();
	sw.Print("ApplyMatrix");
	return sw.ResultMs();
}

function test_NPerspProject() {
	SetProjectionViewport(1, 2, 3, 4);

	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		NPerspProject(i, i, i);
	}
	sw.Stop();
	sw.Print("NPerspProject");
	return sw.ResultMs();
}
function test_PerspProject() {
	SetProjectionViewport(1, 2, 3, 4);

	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		PerspProject(i, i, i);
	}
	sw.Stop();
	sw.Print("PerspProject");
	return sw.ResultMs();
}

function test_NPolygonZNormal() {
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		NPolygonZNormal([i, i, i], [i, i, i], [i, i, i]);
	}
	sw.Stop();
	sw.Print("NPolygonZNormal");
	return sw.ResultMs();
}
function test_PolygonZNormal() {
	var sw = new StopWatch();
	sw.Start();
	for (var i = 0; i < ITERATIONS; i++) {
		PolygonZNormal([i, i, i], [i, i, i], [i, i, i]);
	}
	sw.Stop();
	sw.Print("PolygonZNormal");
	return sw.ResultMs();
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	Stop();
}

function Input(e) {
}

