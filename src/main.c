#define FLAG_IMPLEMENTATION
#include "flag.h"
#define BASE_IMPLEMENTATION
#include "base.h"

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

flag_bool(flag_E, .sym1 = "-E", .desc = "Stop after preprocessing.")
flag_bool(flag_S, .sym1 = "-S", .desc = "Stop after assembly generation.")
flag_bool(flag_C, .sym1 = "-C", .desc = "Stop after object generation. Do not link.")
flag(char*, source_path, .required = true, .arg = "<source_path>", .desc = "Source file to compile")
{
    source_path = flag_shift(*argc, *argv);
    FILE *file = fopen(source_path, "r");
    if (file) {
        fclose(file);
    } else {
        fprintf(stderr, "ERROR: Can not open file \"%s\": %s: %d\n",
            source_path, strerror(errno), errno);
        exit(errno);
    }
}

void
preproc_stage(char *source_path, Buff *file_content)
{
    assert(run_cmd((char*[]){"cc", "-E", "-P", source_path, NULL}, file_content) == 0);
}

int
main(int argc, char *argv[])
{
    if (flag_parse(&argc, &argv)) {
        exit(1);
    }

    Buff file_content = {0};

    preproc_stage(source_path, &file_content); // fills the buffer with preprocessed C code.
    if (flag_E) {
        write(STDOUT_FILENO, file_content.data, file_content.count);
        exit(0);
    }

}
