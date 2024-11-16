#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pdfgen.h"

int LLVMFuzzerTestOneInput(char *data, int size)
{
    char *text;
    struct pdf_doc *pdf;
    int code;

    if (size == 0)
        return 0;

    code = data[0];

    pdf = pdf_create(PDF_A4_WIDTH, PDF_A4_HEIGHT, NULL);
    pdf_set_font(pdf, "Times-Roman");
    pdf_append_page(pdf);
    text = malloc(size);
    memcpy(text, &data[1], size - 1);
    text[size - 1] = '\0';
    pdf_add_barcode(pdf, NULL, code, 0, 0, 300, 10, text, PDF_WHITE);
    free(text);
    pdf_save(pdf, "fuzz.pdf");
    pdf_destroy(pdf);
    return 0;
}
