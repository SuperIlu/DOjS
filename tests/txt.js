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

/*
** This function is called once when the script is started.
*/
function Setup() {
	SetFramerate(30);

	Println("DOJS_ENCODING=" + DOJS_ENCODING);
	SetMissingCharacter(" ");

	def = new Font();
	Println(JSON.stringify(def.ranges));

	f = new Font(JSBOOTPATH + "fonts/tms18.fnt");
	Println(JSON.stringify(f.ranges));
}

// test texts from https://kermitproject.org/utf8.html by Frank da Cruz
var utf8 = [
	"English: The quick brown fox jumps over the lazy dog.",
	"Jamaican: Chruu, a kwik di kwik brong fox a jomp huova di liezi daag de, yu no siit?",
	"Irish: An ḃfuil do ċroí ag bualaḋ ó ḟaitíos an ġrá a ṁeall lena ṗóg éada ó ṡlí do leasa ṫú ? D'ḟuascail Íosa Úrṁac na hÓiġe Beannaiṫe pór Éava agus Áḋaiṁ.",
	"Dutch: Pa's wĳze lynx bezag vroom het fikse aquaduct.",
	"German: Falsches Üben von Xylophonmusik quält jeden größeren Zwerg. (1)",
	"Norwegian: Blåbærsyltetøy ('blueberry jam', includes every extra letter used in Norwegian).",
	"Swedish: Flygande bäckasiner söka strax hwila på mjuka tuvor.",
	"Icelandic: Sævör grét áðan því úlpan var ónýt.",
	"Finnish: (5) Törkylempijävongahdus (This is a perfect pangram, every letter appears only once. Translating it is an art on its own, but I'll say 'rude lover's yelp'. :-D)",
	"Polish: Pchnąć w tę łódź jeża lub osiem skrzyń fig.",
	"Czech: Příliš žluťoučký kůň úpěl ďábelské ódy.",
	"Slovak: Starý kôň na hŕbe kníh žuje tíško povädnuté ruže, na stĺpe sa ďateľ učí kvákať novú ódu o živote.",
	"Slovenian: Šerif bo za domačo vajo spet kuhal žgance.",
	"Sami (Northern): Vuol Ruoŧa geđggiid leat máŋga luosa ja čuovžža.",
	"Hungarian: Árvíztűrő tükörfúrógép.",
	"Spanish: El pingüino Wenceslao hizo kilómetros bajo exhaustiva lluvia y frío, añoraba a su querido cachorro.",
	"Spanish: Volé cigüeña que jamás cruzó París, exhibe flor de kiwi y atún.",
	"Portuguese: O próximo vôo à noite sobre o Atlântico, põe freqüentemente o único médico. (3)",
	"French: Les naïfs ægithales hâtifs pondant à Noël où il gèle sont sûrs d'être déçus en voyant leurs drôles d'œufs abîmés.",
	"Esperanto: Eĥoŝanĝo ĉiuĵaŭde",
	"Esperanto: Laŭ Ludoviko Zamenhof bongustas freŝa ĉeĥa manĝaĵo kun spicoj.",
];

/*
** This function is repeatedly until ESC is pressed or Stop() is called.
*/
function Loop() {
	ClearScreen(EGA.BLACK);

	var c = SizeX() / 2;
	var text = "This is a test of the emergency broadcast system!";

	var yPos = 2;
	for (i = 0; i < utf8.length; i++) {
		TextXY(10, yPos, FromUTF8(utf8[i]), EGA.YELLOW, NO_COLOR);
		yPos += 10;
	}

	TextXY(100, yPos, text, EGA.MAGENTA, NO_COLOR);
	yPos += 10;
	TextXY(20, yPos, "name = " + f.filename + ", height=" + f.height, EGA.WHITE, NO_COLOR);
	yPos += 10;
	TextXY(20, yPos, "width = " + f.StringWidth(text), EGA.WHITE, NO_COLOR);
	yPos += 10;
	TextXY(20, yPos, "height = " + f.StringHeight(text), EGA.WHITE, NO_COLOR);
	yPos += 10;

	f.DrawStringLeft(c, 200, text, EGA.LIGHT_CYAN, NO_COLOR);
	f.DrawStringRight(c, 220, text, EGA.LIGHT_CYAN, NO_COLOR);
	f.DrawStringCenter(c, 240, text, EGA.LIGHT_CYAN, NO_COLOR);
	Line(c, 240 + f.height, c, 200, EGA.WHITE);

	f.DrawStringCenter(c, 300, text, EGA.MAGENTA, NO_COLOR);
	Line(c - 40, 300, c + 40, 300, EGA.GREEN);

	f.DrawStringCenter(c, 330 - f.height, text, EGA.MAGENTA, NO_COLOR);
	Line(c - 40, 330, c + 40, 330, EGA.GREEN);

	f.DrawStringCenter(c, 360 - f.height / 2, text, EGA.MAGENTA, NO_COLOR);
	Line(c - 40, 360, c + 40, 360, EGA.GREEN);
}

/*
** This function is called on any input.
*/
function Input(event) {
}
