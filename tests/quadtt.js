var qt = Require("quadtree");

/*
** This function is called once when the script is started.
*/
function Setup() {
	SetFramerate(30);

	var bounds = {
		x: 0,
		y: 0,
		width: SizeX(),
		height: SizeY()
	}
	var quad = new qt.QuadTree(bounds);

	//insert a random point
	quad.insert({
		x: 12,
		y: 25,
		height: 10,
		width: 25
	});

	var items = quad.retrieve({ x: 11, y: 20, height: 10, width: 20 });

	Println(JSON.stringify(items));
}

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	Stop();
}

/*
** This function is called on any input.
*/
function Input(event) {
}
