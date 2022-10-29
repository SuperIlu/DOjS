#include <stdio.h>
#include <string.h>

#include "pdfgen.h"

int LLVMFuzzerTestOneInput(char *data, int size)
{
    struct pdf_info info;
    if (size > sizeof(info))
        size = sizeof(info);
    memcpy(&info, data, size);
    struct pdf_doc *pdf = pdf_create(PDF_A4_WIDTH, PDF_A4_HEIGHT, &info);
    pdf_set_font(pdf, "Times-Roman");
    pdf_append_page(pdf);
    pdf_save(pdf, "fuzz.pdf");
    pdf_destroy(pdf);
    return 0;
}
