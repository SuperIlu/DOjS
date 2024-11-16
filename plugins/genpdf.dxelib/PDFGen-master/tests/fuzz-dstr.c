#include "pdfgen.c"

int LLVMFuzzerTestOneInput(const char *data, int size)
{
    struct dstr str = INIT_DSTR;
    int len;

    if (size == 0)
        return 0;

    if (data[0] & 0x1) {
        char *new_data = malloc(size);
        if (!new_data)
            return -1;
        memcpy(new_data, data, size);

        // Ensure it's null terminated
        new_data[size - 1] = '\0';
        len = strlen(new_data);

        dstr_append(&str, new_data);
        if (strcmp(dstr_data(&str), new_data) != 0) {
            printf("Comparison failed:\n%s\n%s\n", dstr_data(&str), new_data);
            return -1;
        }
        free(new_data);
    } else {
        dstr_append_data(&str, data, size);
        if (memcmp(dstr_data(&str), data, size) != 0) {
            printf("Binary compare failed\n");
            return -1;
        }
    }
    dstr_free(&str);
    return 0;
}
