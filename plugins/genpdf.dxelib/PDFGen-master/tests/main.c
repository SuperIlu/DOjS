#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "pdfgen.h"

extern unsigned char data_penguin_jpg[];
extern unsigned int data_penguin_jpg_len;

extern unsigned char data_rgb[];

int main(int argc, char *argv[])
{
    struct pdf_info info = {.creator = "My software",
                            .producer = "My software",
                            .title = "My document",
                            .author = "My name",
                            .subject = "My subject"};
    struct pdf_doc *pdf = pdf_create(PDF_A4_WIDTH, PDF_A4_HEIGHT, &info);
    struct pdf_object *first_page, *second_page;
    int i;
    float height, width;
    int bm;
    int err;

    setlocale(LC_ALL, "");

    /* Unused */
    (void)argc;
    (void)argv;

    if (!pdf) {
        fprintf(stderr, "Unable to create PDF\n");
        return -1;
    }

    if (pdf_width(pdf) != PDF_A4_WIDTH || pdf_height(pdf) != PDF_A4_HEIGHT) {
        fprintf(stderr, "PDF Size mismatch: %fx%f\n", pdf_width(pdf),
                pdf_height(pdf));
        return -1;
    }

    err = pdf_get_font_text_width(pdf, "Times-BoldItalic", "foo", 14, &width);
    if (err < 0 || width < 18) {
        fprintf(stderr, "Font width invalid: %d/%f\n", err, width);
        return -1;
    }

    /* These calls should fail, since we haven't added a page yet */
    if (pdf_add_image_file(pdf, NULL, 10, 10, 20, 30, "data/teapot.ppm") >= 0)
        return -1;

    if (pdf_add_image_file(pdf, NULL, 100, 500, 50, 150,
                           "data/penguin.jpg") >= 0)
        return -1;

    if (pdf_add_image_file(pdf, NULL, 200, 500, 100, 100, "data/coal.png") >=
        0)
        return -1;

    if (pdf_add_image_file(pdf, NULL, 300, 500, 243, 204, "data/bee.bmp") >=
        0)
        return -1;

    if (pdf_add_text(pdf, NULL, "Page One", 10, 20, 30,
                     PDF_RGB(0xff, 0, 0)) >= 0)
        return -1;

    if (pdf_add_bookmark(pdf, NULL, -1, "Another Page") >= 0)
        return -1;

    if (!pdf_get_err(pdf, &err))
        return -1;

    pdf_clear_err(pdf);
    /* From now on, we shouldn't see any errors */

    pdf_set_font(pdf, "Times-BoldItalic");
    pdf_append_page(pdf);
    first_page = pdf_get_page(pdf, 1);

    pdf_add_text_wrap(
        pdf, NULL,
        "This is a great big long string that I hope will wrap properly "
        "around several lines.\nThere are some odd length "
        "linesthatincludelongwords to check the justification. "
        "I've put some embedded line breaks in to "
        "see how it copes with them. Hopefully it all works properly.\n\n\n"
        "We even include multiple breaks\n"
        "And special stuff €ÜŽžŠšÁ that áüöä should ÄÜÖß— “”‘’ break\n"
        "————————————————————————————————————————————————\n"
        "thisisanenourmouswordthatwillneverfitandwillhavetobecut",
        16, 60, 800, 0, PDF_RGB(0, 0, 0), 300, PDF_ALIGN_JUSTIFY, &height);
    pdf_add_rectangle(pdf, NULL, 58, 800 + 16, 304, -height, 2,
                      PDF_RGB(0, 0, 0));
    pdf_add_image_file(pdf, NULL, 10, 10, 20, 30, "data/teapot.ppm");
    pdf_add_image_file(pdf, NULL, 50, 10, 30, 30, "data/coal.png");
    pdf_add_image_file(pdf, NULL, 100, 10, 30, 30, "data/bee.bmp");
    pdf_add_image_file(pdf, NULL, 150, 10, 30, 30, "data/bee-32-flip.bmp");

    pdf_add_image_file(pdf, NULL, 150, 50, 50, 150, "data/grey.jpg");
    pdf_add_image_file(pdf, NULL, 200, 50, 50, -1, "data/bee.pgm");
    pdf_add_image_file(pdf, NULL, 400, 100, 100, 100, "data/grey.png");
    pdf_add_image_file(pdf, NULL, 400, 210, 100, 100, "data/indexed.png");

    pdf_add_image_data(pdf, NULL, 100, 500, 50, 150, data_penguin_jpg,
                       data_penguin_jpg_len);

    pdf_add_text(pdf, NULL, "Page One", 10, 20, 30, PDF_RGB(0xff, 0, 0));
    pdf_add_text(pdf, NULL, "PjGQji", 18, 20, 130, PDF_RGB(0, 0xff, 0xff));
    pdf_add_line(pdf, NULL, 10, 24, 100, 24, 4, PDF_RGB(0xff, 0, 0));
    pdf_add_cubic_bezier(pdf, NULL, 10, 100, 150, 100, 20, 30, 60, 30, 4,
                         PDF_RGB(0, 0xff, 0));
    pdf_add_quadratic_bezier(pdf, NULL, 10, 140, 150, 140, 50, 160, 4,
                             PDF_RGB(0, 0, 0xff));
    struct pdf_path_operation operations[] = {
        {.op = 'm', .x1 = 100, .y1 = 100},
        {.op = 'l', .x1 = 130, .y1 = 100},
        {.op = 'c',
         .x1 = 150,
         .y1 = 150,
         .x2 = 100,
         .y2 = 100,
         .x3 = 130,
         .y3 = 130},
        {.op = 'l', .x1 = 150, .y1 = 120},
        {.op = 'h'},
    };
    int operation_count = (sizeof(operations) / sizeof((operations)[0]));
    pdf_add_custom_path(pdf, NULL, operations, operation_count, 1,
                        PDF_RGB(0xff, 0, 0), PDF_ARGB(0x80, 0xff, 0, 0));
    pdf_add_circle(pdf, NULL, 100, 240, 50, 5, PDF_RGB(0xff, 0, 0),
                   PDF_TRANSPARENT);
    pdf_add_ellipse(pdf, NULL, 100, 240, 40, 30, 2, PDF_RGB(0xff, 0xff, 0),
                    PDF_RGB(0, 0, 0));
    pdf_add_rectangle(pdf, NULL, 150, 150, 100, 100, 4, PDF_RGB(0, 0, 0xff));
    pdf_add_filled_rectangle(pdf, NULL, 150, 450, 100, 100, 4,
                             PDF_RGB(0, 0xff, 0), PDF_TRANSPARENT);
    pdf_add_text_rotate(pdf, NULL, "This should be transparent", 20, 160, 500,
                        M_PI / 4, PDF_ARGB(0x80, 0, 0, 0));

    float p1X[] = {200, 200, 300, 300};
    float p1Y[] = {200, 300, 200, 300};
    pdf_add_polygon(pdf, NULL, p1X, p1Y, 4, 4, PDF_RGB(0xaa, 0xff, 0xee));
    float p2X[] = {400, 400, 500, 500};
    float p2Y[] = {400, 500, 400, 500};
    pdf_add_filled_polygon(pdf, NULL, p2X, p2Y, 4, 4,
                           PDF_RGB(0xff, 0x77, 0x77));
    pdf_add_text(pdf, NULL, "", 20, 20, 30, PDF_RGB(0, 0, 0));
    pdf_add_text(pdf, NULL, "Date (YYYY-MM-DD):", 20, 220, 30,
                 PDF_RGB(0, 0, 0));

    pdf_add_bookmark(pdf, NULL, -1, "First page");

    pdf_append_page(pdf);
    second_page = pdf_get_page(pdf, 2);

    pdf_add_text(pdf, second_page, "Page Two", 10, 20, 30, PDF_RGB(0, 0, 0));
    pdf_add_text(pdf, NULL, "This is some weird text () \\ # : - Wi-Fi 27°C",
                 10, 50, 60, PDF_RGB(0, 0, 0));
    pdf_add_text(
        pdf, NULL,
        "Control characters ( ) < > [ ] { } / % \n \r \t \b \f ending", 10,
        50, 45, PDF_RGB(0, 0, 0));
    pdf_add_text(pdf, NULL, "Special characters: €ÜŽžŠšÁáüöäÄÜÖß—“”‘’Æ", 10,
                 50, 15, PDF_RGB(0, 0, 0));
    pdf_add_text(pdf, NULL, "This one has a new line in it\nThere it was", 10,
                 50, 80, PDF_RGB(0, 0, 0));
    pdf_add_text(
        pdf, NULL,
        "This is a really long line that will go off the edge of the screen, "
        "because it is so long. I like long text. The quick brown fox jumped "
        "over the lazy dog. The quick brown fox jumped over the lazy dog",
        10, 100, 100, PDF_RGB(0, 0, 0));
    pdf_set_font(pdf, "Helvetica-Bold");
    pdf_add_text(
        pdf, NULL,
        "This is a really long line that will go off the edge of the screen, "
        "because it is so long. I like long text. The quick brown fox jumped "
        "over the lazy dog. The quick brown fox jumped over the lazy dog",
        10, 100, 130, PDF_RGB(0, 0, 0));
    pdf_set_font(pdf, "ZapfDingbats");
    pdf_add_text(
        pdf, NULL,
        "This is a really long line that will go off the edge of the screen, "
        "because it is so long. I like long text. The quick brown fox jumped "
        "over the lazy dog. The quick brown fox jumped over the lazy dog",
        10, 100, 150, PDF_RGB(0, 0, 0));

    pdf_set_font(pdf, "Courier-Bold");
    pdf_add_text(pdf, NULL, "(5.6.5) RS232 shutdown", 8, 317, 546,
                 PDF_RGB(0, 0, 0));
    pdf_add_text(pdf, NULL, "", 8, 437, 546, PDF_RGB(0, 0, 0));
    pdf_add_text(pdf, NULL, "Pass", 8, 567, 556, PDF_RGB(0, 0, 0));
    pdf_add_text(pdf, NULL, "(5.6.3) RS485 pins", 8, 317, 556,
                 PDF_RGB(0, 0, 0));

    bm = pdf_add_bookmark(pdf, NULL, -1, "Another Page");
    bm = pdf_add_bookmark(pdf, NULL, bm, "Another Page again");
    pdf_add_bookmark(pdf, NULL, bm, "A child page");
    pdf_add_bookmark(pdf, NULL, bm, "Another child page");
    pdf_add_bookmark(pdf, NULL, -1, "Top level again");
    pdf_append_page(pdf);

    pdf_set_font(pdf, "Times-Roman");
    for (i = 0; i < 3000; i++) {
        float xpos = (i / 100) * 40.0f;
        float ypos = (i % 100) * 10.0f;
        pdf_add_text(pdf, NULL, "Text blob", 8, xpos, ypos,
                     PDF_RGB(i, (i * 4) & 0xff, (i * 8) & 0xff));
    }
    pdf_add_text(pdf, NULL, "", 10, (i / 100) * 100, (i % 100) * 12,
                 PDF_RGB(0xff, 0, 0));

    struct pdf_object *page = pdf_append_page(pdf);

    if (pdf_page_width(page) != PDF_A4_WIDTH ||
        pdf_page_height(page) != PDF_A4_HEIGHT) {
        fprintf(stderr, "PDF Size mismatch: %fx%f\n", pdf_page_width(page),
                pdf_page_height(page));
        return -1;
    }

    pdf_add_barcode(pdf, NULL, PDF_BARCODE_39, PDF_MM_TO_POINT(20),
                    PDF_MM_TO_POINT(240), PDF_MM_TO_POINT(60),
                    PDF_MM_TO_POINT(20), "CODE39", PDF_RGB(0, 0, 0));

    pdf_add_barcode(pdf, NULL, PDF_BARCODE_128A, PDF_MM_TO_POINT(20),
                    PDF_MM_TO_POINT(210), PDF_MM_TO_POINT(60),
                    PDF_MM_TO_POINT(20), "Code128", PDF_RGB(0, 0, 0));

    pdf_add_text_wrap(pdf, NULL, "EAN13 Barcode", 10, PDF_MM_TO_POINT(20),
                      PDF_MM_TO_POINT(155), 0, PDF_RGB(0, 0, 0),
                      PDF_MM_TO_POINT(60), PDF_ALIGN_CENTER, NULL);
    pdf_add_rectangle(pdf, NULL, PDF_MM_TO_POINT(20), PDF_MM_TO_POINT(160),
                      PDF_MM_TO_POINT(60), PDF_MM_TO_POINT(40), 1,
                      PDF_RGB(0xff, 0, 0xff));
    pdf_add_barcode(pdf, NULL, PDF_BARCODE_EAN13, PDF_MM_TO_POINT(20),
                    PDF_MM_TO_POINT(160), PDF_MM_TO_POINT(60),
                    PDF_MM_TO_POINT(40), "4003994155486", PDF_BLACK);
    pdf_add_text_wrap(pdf, NULL, "UPCA Barcode", 10, PDF_MM_TO_POINT(100),
                      PDF_MM_TO_POINT(155), 0, PDF_RGB(0, 0, 0),
                      PDF_MM_TO_POINT(60), PDF_ALIGN_CENTER, NULL);
    pdf_add_rectangle(pdf, NULL, PDF_MM_TO_POINT(100), PDF_MM_TO_POINT(160),
                      PDF_MM_TO_POINT(60), PDF_MM_TO_POINT(80), 1,
                      PDF_RGB(0, 0, 0xff));
    pdf_add_barcode(pdf, NULL, PDF_BARCODE_UPCA, PDF_MM_TO_POINT(100),
                    PDF_MM_TO_POINT(160), PDF_MM_TO_POINT(60),
                    PDF_MM_TO_POINT(80), "003994155480", PDF_BLACK);

    pdf_add_text_wrap(pdf, NULL, "EAN8 Barcode", 10, PDF_MM_TO_POINT(20),
                      PDF_MM_TO_POINT(55), 0, PDF_RGB(0, 0, 0),
                      PDF_MM_TO_POINT(60), PDF_ALIGN_CENTER, NULL);
    pdf_add_rectangle(pdf, NULL, PDF_MM_TO_POINT(20), PDF_MM_TO_POINT(60),
                      PDF_MM_TO_POINT(60), PDF_MM_TO_POINT(40), 1,
                      PDF_RGB(0, 0xff, 0xff));
    pdf_add_barcode(pdf, NULL, PDF_BARCODE_EAN8, PDF_MM_TO_POINT(20),
                    PDF_MM_TO_POINT(60), PDF_MM_TO_POINT(60),
                    PDF_MM_TO_POINT(40), "95012346", PDF_BLACK);
    pdf_add_text_wrap(pdf, NULL, "UPCE Barcode", 10, PDF_MM_TO_POINT(100),
                      PDF_MM_TO_POINT(55), 0, PDF_RGB(0, 0, 0),
                      PDF_MM_TO_POINT(60), PDF_ALIGN_CENTER, NULL);
    pdf_add_rectangle(pdf, NULL, PDF_MM_TO_POINT(100), PDF_MM_TO_POINT(60),
                      PDF_MM_TO_POINT(60), PDF_MM_TO_POINT(80), 1,
                      PDF_RGB(0, 0xff, 0));
    pdf_add_barcode(pdf, NULL, PDF_BARCODE_UPCE, PDF_MM_TO_POINT(100),
                    PDF_MM_TO_POINT(60), PDF_MM_TO_POINT(60),
                    PDF_MM_TO_POINT(80), "012345000058", PDF_BLACK);

    pdf_append_page(pdf);
    pdf_page_set_size(pdf, NULL, PDF_A3_HEIGHT, PDF_A3_WIDTH);
    pdf_add_bookmark(pdf, NULL, -1, "Last Page");

    pdf_add_text(pdf, NULL, "This is an A3 landscape page", 10, 20, 30,
                 PDF_RGB(0xff, 0, 0));
    if (pdf_get_font_text_width(pdf, NULL, "This is an A3 landscape page", 10,
                                &width) == 0) {
        pdf_add_link(pdf, NULL, 20, 30, width, 10, first_page, 0,
                     pdf_page_height(first_page) / 2);
    }
    pdf_add_rgb24(pdf, NULL, 72, 72, 288, 144, data_rgb, 16, 8);

    pdf_save(pdf, "output.pdf");

    const char *err_str = pdf_get_err(pdf, &err);
    if (err_str) {
        fprintf(stderr, "PDF Error: %d - %s\n", err, err_str);
        pdf_destroy(pdf);
        return -1;
    }
    pdf_destroy(pdf);

    return 0;
}
