#ifndef _FLAG_H
#define _FLAG_H

struct Flag {
    char* sym1;
    char* sym2;
    char* arg;
    char* des;
    _Bool is_set;
    _Bool required;
    void (*handle)(int*, char**[], struct Flag*);

    _Bool stop;
};

#define __funused __attribute__((unused))

#define flag_shift(xs_sz, xs) ((xs_sz)--, *(xs)++)

// Strange thing is that this struct of size 24 bytes is not automatically
// alined if you create a list of it or increment a pointer they both move by 24
// byte steps! But when you define variables of them on a custom section they
// are put into the section with 32 byte alignment!
// Solution is to set alignment for variables when defined.
#define flag(type, name, required, sym1, sym2, arg, des)       \
    void name##_h(int*, char**[], struct Flag*);     \
    __attribute__((section ("flags_section")))       \
    __attribute__((aligned (_Alignof(struct Flag)))) \
    struct Flag flag_##name = {                      \
         sym1,                                       \
         sym2,                                       \
         arg,                                        \
         des,                                        \
         0,                                          \
         required,                                   \
         name##_h,                                   \
                                                     \
         0,                                          \
    };                                               \
    type name;                                       \
    void name##_h (__funused int* argc, __funused char** argv[], \
        __funused struct Flag *f)

#endif // _FLAG_H

#ifdef FLAG_IMPLEMENTATION
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define flag_list(a) (&__start_flags_section + a)
#define flag_list_end() &__stop_flags_section
extern struct Flag __start_flags_section;
extern struct Flag __stop_flags_section;

const char *program_name;

flag(_Bool, help, false, "-h", "--help", "", "Print Help") {
    f->stop = 1;

    int longest_tab = 0;
    for(struct Flag *l = flag_list(0); l != flag_list_end(); ++l) {
        int flag_len = strlen(l->sym1) + strlen(l->sym2) + strlen(l->arg);
        if (longest_tab < flag_len) {
            longest_tab = flag_len;
        }
    }

    for(struct Flag *l = flag_list(0); l != flag_list_end(); ++l) {
        int tab = longest_tab - (1 + strlen(l->sym1) + strlen(l->sym2) + strlen(l->arg));
        char *comma = l->sym2[0] != '\0' ? "," : "";
        printf("%s%s%s %s%*c\t%s\n", l->sym1, comma, l->sym2, l->arg, tab, ' ', l->des);
    }
}

int flag_parse(int *argc, char **argv[]) {
    program_name = flag_shift(*argc, *argv);
    while(*argc > 0) {
        const char* token = flag_shift(*argc, *argv);
        struct Flag *l;
        for(l = flag_list(0); l != flag_list_end(); ++l) {
            if ((strcmp(l->sym1, token) == 0 || strcmp(l->sym2, token) == 0) ||
                (token[0] != '-' && (l->sym1[0] == '\0' && l->sym2[0] == '\0')))
            {
                l->is_set = 1;
                l->handle(argc, argv, l);
                if (l->stop) return l->stop;
                break;
            }
        }
        if (l == flag_list_end()) {
            fprintf(stderr, "Unknow flag \"%s\"\n", token);
            return -1;
        }
    }

    for(struct Flag *l = flag_list(0); l != flag_list_end(); ++l) {
        if (l->required && !l->is_set) {
            char *comma = l->sym2[0] != '\0' ? "," : "";
            fprintf(stderr, "%s%s%s %s is required\n", l->sym1, comma, l->sym2, l->arg);
            return -1;
        }
    }

    return 0;
}
#endif
