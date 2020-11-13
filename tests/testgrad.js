Include('p5');

var pics;

function setup() {
	pics = [
		loadImage("tests/testgrad.png"),
		loadImage("tests/testgrad.tga"),
		loadImage("tests/testgrad.bmp"),
		loadImage("tests/testgrad.pcx")
	];
}

function draw() {
	for (var p = 0; p < pics.length; p++) {
		var pic = pics[p];
		print(pic.width + "x" + pic.height);

		for (var x = 0; x < pic.width; x++) {
			println(p + " color=" + pic.get(x, 0));
			println(p + " blue=" + blue(pic.get(x, 0)));
		}
	}
	noLoop();
}
