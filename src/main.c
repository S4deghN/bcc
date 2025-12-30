#define FLAG_IMPLEMENTATION
#include "flag.h"
#include "utils.h"

#include <stdio.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>

flag(bool, flag_E, false, "-E", "", "", "Stop after preprocessing.")
{
    flag_E = true;
}

flag(bool, flag_S, false, "-S", "", "", "Stop after assembly generation.")
{
    flag_S = true;
}

flag(bool, flag_C, false, "-C", "", "", "Stop after object generation. Do not link.")
{
    flag_C = true;
}

flag(char*, source_path, true, "", "", "<source_path>", "Source file to compile")
{
    source_path = flag_shift(*argc, *argv);
    FILE *file = fopen(source_path, "r");
    if (file) {
        fclose(file);
    } else {
        fprintf(stderr, "ERROR: Can not open file \"%s\": %s: %d\n",
            source_path, strerror(errno), errno);
        flag_exit(errno);
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

    preproc_stage(source_path, &file_content);

    if (flag_E) {
        write(STDOUT_FILENO, file_content.data, file_content.count);
        exit(0);
    }
}
