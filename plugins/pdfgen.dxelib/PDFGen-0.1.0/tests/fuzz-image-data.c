#include <stdio.h>
#include <string.h>

#include "pdfgen.h"

#define filename "./fuzz.png"
int LLVMFuzzerTestOneInput(char *data, int size)
{
    struct pdf_doc *pdf = pdf_create(PDF_A4_WIDTH, PDF_A4_HEIGHT, NULL);
    pdf_set_font(pdf, "Times-Roman");
    pdf_append_page(pdf);
    pdf_add_image_data(pdf, NULL, 100, 500, 50, 150, (uint8_t *)data, size);
    pdf_save(pdf, "fuzz-image-data.pdf");
    pdf_destroy(pdf);
    return 0;
}
