var foo = 2;
var cnt = 5;

/*
** This function is called once when the script is started.
*/
function Setup() {
	SetFramerate(500);

	Println("\nGetRotationMatrix:");
	Println(JSON.stringify(GetRotationMatrix(10, 20, 30)));
	Println(JSON.stringify(NGetRotationMatrix(10, 20, 30)));

	Println("\nGetTransformationMatrix:");
	Println(JSON.stringify(GetTransformationMatrix(42, 10, 20, 30, 40, 50, 60)));
	Println(JSON.stringify(NGetTransformationMatrix(42, 10, 20, 30, 40, 50, 60)));

	Println("\nGetXRotateMatrix:");
	Println(JSON.stringify(GetXRotateMatrix(10)));
	Println(JSON.stringify(NGetXRotateMatrix(10)));

	Println("\nGetYRotateMatrix:");
	Println(JSON.stringify(GetYRotateMatrix(10)));
	Println(JSON.stringify(NGetYRotateMatrix(10)));

	Println("\nGetZRotateMatrix:");
	Println(JSON.stringify(GetZRotateMatrix(10)));
	Println(JSON.stringify(NGetZRotateMatrix(10)));

	Println("\nMatrixMul:");
	Println(JSON.stringify(MatrixMul(GetZRotateMatrix(10), GetXRotateMatrix(10))));
	Println(JSON.stringify(NMatrixMul(NGetZRotateMatrix(10), NGetXRotateMatrix(10))));

	Println("\nApplyMatrix:");
	Println(JSON.stringify(ApplyMatrix(GetTransformationMatrix(42, 10, 20, 30, 40, 50, 60), [1, 2, 3, 4, 5])));
	Println(JSON.stringify(NApplyMatrix(GetTransformationMatrix(42, 10, 20, 30, 40, 50, 60), [1, 2, 3, 4, 5])));
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	ClearScreen(EGA.BLACK);
	var vec = [1, 2, 3, 4, 5];
	var start = MsecTime();
	if (foo == "1") {
		for (var i = 0; i < cnt; i++) {
			GetRotationMatrix(10, 20, 30);
			var txm = GetTransformationMatrix(42, 10, 20, 30, 40, 50, 60);
			var rotx = GetXRotateMatrix(10);
			var roty = GetYRotateMatrix(10);
			GetZRotateMatrix(10);
			MatrixMul(rotx, roty);
			ApplyMatrix(txm, vec)
			PerspProject(vec);
			PolygonZNormal(vec, vec, vec);
		}
	} else {
		for (var i = 0; i < cnt; i++) {
			NGetRotationMatrix(10, 20, 30);
			var txm = NGetTransformationMatrix(42, 10, 20, 30, 40, 50, 60);
			var rotx = NGetXRotateMatrix(10);
			var roty = NGetYRotateMatrix(10);
			NGetZRotateMatrix(10);
			NMatrixMul(rotx, roty);
			NApplyMatrix(txm, vec)
			NPerspProject(vec);
			NPolygonZNormal(vec, vec, vec);
		}
	}
	var end = MsecTime();
	Println(foo + " time=" + (end - start));

	TextXY(10, 10, foo + " rate=" + GetFramerate(), EGA.LIGHT_BLUE);
}

/*
** This function is called on any input.
*/
function Input(e) {
	if (CompareKey(e.key, '1')) {
		foo = 1;
		Println("To ONE");
	}
	if (CompareKey(e.key, '2')) {
		foo = 2;
		Println("To TWO");
	}
}
