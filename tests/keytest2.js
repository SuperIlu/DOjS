LoadLibrary("png");

var xPos = 160;
var yPos = 120;
var speed = 2; // Adjust the speed according to your preference

var sprite;
var bck;

function Setup() {
	MouseShowCursor(false);
	sprite = new Bitmap("tests/testdata/sonic.png");
	bck = new Bitmap("tests/testdata/bckgrnd.png");
}

function Loop() {
	// ClearScreen(EGA.BLACK); // Clear the canvas
	bck.Draw(0, 0);

	sprite.DrawTrans(xPos, yPos);

	// Move the pixel when keys are held down
	if (KeyIsPressed(KEY.Code.KEY_UP)) {
		yPos -= speed;
	}
	if (KeyIsPressed(KEY.Code.KEY_DOWN)) {
		yPos += speed;
	}
	if (KeyIsPressed(KEY.Code.KEY_LEFT)) {
		xPos -= speed;
	}
	if (KeyIsPressed(KEY.Code.KEY_RIGHT)) {
		xPos += speed;
	}
}
