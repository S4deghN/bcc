#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

#define FLAG_IMPLEMENTATION
#include "flag.h"

flag(bool, flag_E, false, "-E", "", "", "Stop after preprocessing.") {
    flag_E = true;
}

flag(bool, flag_S, false, "-S", "", "", "Stop after assembly generation.") {
    flag_S = true;
}

flag(bool, flag_C, false, "-C", "", "", "Stop after object generation. Do not link.") {
    flag_C = true;
}

flag(FILE*, source_file, true, "", "", "<source_path>", "Source file to compile") {
    char *source_path = *(*argv-1);
    source_file = fopen(source_path, "r");
    if (!source_file) {
        f->stop = errno;
        fprintf(stderr, "ERROR: Can not open file \"%s\": %s: %d\n", 
            source_path, strerror(errno), errno);
    }
}

void preproc_stateg() {
    printf("TODO: Implement preproc\n");
}

int
main(int argc, char *argv[])
{
    int stop = flag_parse(&argc, &argv);
    if (stop) {
        exit(stop > 0 ? 0 : -stop);
    }

    preproc_stateg();
    if (flag_E) {
        printf("exiting after preproc\n");
        exit(0);
    }
}
