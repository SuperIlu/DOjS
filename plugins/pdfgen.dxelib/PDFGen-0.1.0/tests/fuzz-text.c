#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pdfgen.h"

int LLVMFuzzerTestOneInput(char *data, int size)
{
    char *text;
    struct pdf_doc *pdf = pdf_create(PDF_A4_WIDTH, PDF_A4_HEIGHT, NULL);
    pdf_set_font(pdf, "Times-Roman");
    pdf_append_page(pdf);
    text = malloc(size + 1);
    memcpy(text, data, size);
    text[size] = '\0';
    pdf_add_text(pdf, NULL, text, 10, 20, 30, PDF_RGB(0xff, 0, 0));
    pdf_add_text_wrap(pdf, NULL, text, 10, 20, 30, PDF_RGB(0xff, 0, 0), 200,
                      PDF_ALIGN_LEFT, NULL);
    free(text);
    pdf_save(pdf, "fuzz.pdf");
    pdf_destroy(pdf);
    return 0;
}
