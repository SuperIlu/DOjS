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

LoadLibrary("pdfgen");

function Setup() {
    Println(PDF_A4_WIDTH);
    Println(PDF_A4_HEIGHT);
    Println(PDF_TIMES);
    // struct pdf_doc *pdf = pdf_create(PDF_A4_WIDTH, PDF_A4_HEIGHT, &info);
    pdf = new PDFGen(PDF_A4_WIDTH, PDF_A4_HEIGHT);

    Println("width=" + pdf.width);
    Println("height=" + pdf.height);

    // pdf_set_font(pdf, "Times-Roman");
    pdf.SetFont(PDF_TIMES);
    //pdf_append_page(pdf);
    pdf.AppendPage();
    //pdf_add_text(pdf, NULL, "This is text", 12, 50, 20, PDF_BLACK);
    pdf.AddText("This is text", 12, 150, 20, EGA.BLACK);
    // pdf_add_text(pdf, NULL, "Page One", 10, 20, 30, PDF_RGB(0xff, 0, 0));
    pdf.AddText("Page One", 10, 120, 30, EGA.BLUE);
    //pdf_add_line(pdf, NULL, 50, 24, 150, 24);
    pdf.AddLine(50, 24, 150, 24, 5, EGA.RED);
    pdf.AddLine(50, 20, 150, 20, 5, EGA.GREEN);
    pdf.AddLine(10, 24, 100, 24, 4, EGA.BLUE);

    pdf.AddCubicBezier(10, 100, 150, 100, 20, 30, 60, 30, 4, EGA.LIGHT_GREEN);
    pdf.AddQuadraticBezier(10, 140, 150, 140, 50, 160, 4, EGA.LIGHT_BLUE);

    var bmid = pdf.AddBookmark(PDF_TOPLEVEL_BOOKMARK, "First page");
    var bmid2 = pdf.AddBookmark(bmid, "First sublevel");

    pdf.AppendPage();
    height = pdf.AddTextWrap(
        "This is a great big long string that I hope will wrap properly " +
        "around several lines.\nThere are some odd length " +
        "linesthatincludelongwords to check the justification. " +
        "I've put some embedded line breaks in to " +
        "see how it copes with them. Hopefully it all works properly.\n\n\n" +
        "We even include multiple breaks\n" +
        "thisisanenourmouswordthatwillneverfitandwillhavetobecut",
        16, 60, 800, EGA.BLACK, 300, PDF_ALIGN_JUSTIFY);
    pdf.AddRectangle(58, 800 + 16, 304, -height, 2, EGA.BLACK);

    pdf.AppendPage();
    pdf.AddImageFile(10, 10, 20, 30, "tests/teapot.ppm");
    pdf.AddImageFile(50, 10, 30, 30, "tests/coal.png");
    pdf.AddImageFile(100, 10, 30, 30, "tests/bee.bmp");
    pdf.AddImageFile(150, 10, 30, 30, "tests/bee32.bmp");
    pdf.AddImageFile(150, 50, 50, 150, "tests/grey.jpg");

    // create bitmap with X in it and render it to the PDF
    pdf.AppendPage();
    bm = new Bitmap(100, 100);
    SetRenderBitmap(bm);
    ClearScreen(EGA.BLACK);
    Line(0, 0, 100, 100, EGA.RED);
    Line(100, 0, 0, 100, EGA.GREEN);
    SetRenderBitmap(null);
    pdf.AddRgb(250, 200, 100, 100, bm);

    pdf.AppendPage();
    pdf.AddBarcode(PDF_BARCODE_128A, 50, 300, 200, 50, "Code128", EGA.BLACK);
    pdf.AddBarcode(PDF_BARCODE_39, 50, 400, 400, 50, "CODE39", EGA.BLACK);

    pdf.AppendPage();
    pdf.AddCircle(200, 240, 50, 5, EGA.RED, NO_COLOR);
    pdf.AddEllipse(100, 240, 40, 30, 2, EGA.YELLOW, EGA.BROWN);
    pdf.AddRectangle(150, 150, 100, 100, 4, EGA.BLUE);
    pdf.AddFilledRectangle(150, 450, 100, 100, 4, EGA.GREEN);

    pdf.AppendPage();
    var p = [
        [200, 200],
        [200, 300],
        [300, 200],
        [300, 300]
    ];
    pdf.AddPolygon(p, 4, Color(0xaa, 0xff, 0xee));
    var fp = [
        [400, 400],
        [400, 500],
        [500, 400],
        [500, 500]
    ];
    pdf.AddFilledPolygon(fp, 4, Color(0xff, 0x77, 0x77));

    pdf.AppendPage();
    var ops = [
        ["m", 100, 100, 0, 0, 0, 0],
        ["l", 130, 100, 0, 0, 0, 0],
        ["c", 150, 150, 100, 100, 130, 130],
        ["l", 150, 120, 0, 0, 0, 0],
        ["h", 0, 0, 0, 0, 0, 0]
    ];
    pdf.AddCustomPath(ops, 1, Color(0xff, 0, 0), Color(0x80, 0xff, 0));

    // pdf_save(pdf, "output.pdf");
    pdf.Save("output.pdf");
    // pdf_destroy(pdf);
    pdf = null;

    Stop();
}

function Loop() {
}

function Input(e) {
}
