#ifndef _FLAG_H
#define _FLAG_H

#if !defined(__unused)
#define __unused __attribute__((unused))
#endif

#define flag_shift(xs_sz, xs) ((xs_sz)--, *(xs)++)

struct Flag {
    char* sym1;
    char* sym2;
    char* arg;
    char* desc;
    _Bool required;
    _Bool is_set;
    void (*handle)(int*, char**[], struct Flag*);
};

// Strange thing is that this struct of size 24 bytes is not automatically
// alined if you create a list of it or increment a pointer they both move by 24
// byte steps! But when you define variables of them on a custom section they
// are put into the section with 32 byte alignment!
// Solution is to set alignment for variables when defined.
#define flag(type, name, ...)                                    \
    void name##_h(int*, char**[], struct Flag*);                 \
    __attribute__((section ("flags_section")))                   \
    __attribute__((aligned (_Alignof(struct Flag))))             \
    struct Flag flag_##name = {                                  \
        __VA_ARGS__,                                             \
        .is_set = 0,                                             \
        .handle = name##_h,                                      \
    };                                                           \
    type name;                                                   \
    void name##_h (__unused int* argc, __unused char** argv[], \
        __unused struct Flag *f)

#define flag_bool(name, ...) flag(_Bool, name, __VA_ARGS__) { \
    name = 1; \
}

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

size_t flag_strlen(const char *str) {
    if (!str) return 0;
    return strlen(str);
}

int flag_strcmp(const char *s1, const char *s2) {
    if (s1 == s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;
    return strcmp(s1, s2);
}

void flag_fprint_flag(FILE *f, struct Flag *l) {
    char *comma = l->sym2 != NULL ? "," : "";
    char *space = (l->sym1 != NULL || l->sym2 != NULL) && l->arg != NULL ? " " : "";
    char *sym1  = l->sym1 ? l->sym1 : "";
    char *sym2  = l->sym2 ? l->sym2 : "";
    char *arg   = l->arg ? l->arg : "";
    fprintf(f, "%s%s%s%s%s", sym1, comma, sym2 , space, arg);
}

flag(_Bool, help, .sym1 = "-h", .sym2 = "--help", .desc = "Print Help") {
    int longest_tab = 0;
    for(struct Flag *l = flag_list(0); l != flag_list_end(); ++l) {
        int flag_len = flag_strlen(l->sym1) + flag_strlen(l->sym2) + flag_strlen(l->arg);
        if (longest_tab < flag_len) {
            longest_tab = flag_len;
        }
    }

    for(struct Flag *l = flag_list(0); l != flag_list_end(); ++l) {
        int tab = longest_tab - (1 + flag_strlen(l->sym1) + flag_strlen(l->sym2) + flag_strlen(l->arg));
        flag_fprint_flag(stdout, l);
        printf("%*c\t%s\n", tab, ' ', l->desc);
    }

    exit(0);
}

int flag_parse(int *argc, char **argv[]) {
    program_name = flag_shift(*argc, *argv);
    while(*argc > 0) {
        const char* token = flag_shift(*argc, *argv);
        struct Flag *l;
        for(l = flag_list(0); l != flag_list_end(); ++l) {
            if ((flag_strcmp(l->sym1, token) == 0 || flag_strcmp(l->sym2, token) == 0) ||
                (token[0] != '-' && (l->sym1 == NULL && l->sym2 == NULL)))
            {
                // In case of an empty symbol restore the cosumed arg
                if (token[0] != '-') { ++(*argc); --(*argv); }
                l->is_set = 1;
                l->handle(argc, argv, l);
                break;
            }
        }
        if (l == flag_list_end()) {
            fprintf(stderr, "Unknow flag \"%s\"\n", token);
            return 1;
        }
    }

    _Bool some_was_not_set = 0;
    for(struct Flag *l = flag_list(0); l != flag_list_end(); ++l) {
        if (l->required && !l->is_set) {
            flag_fprint_flag(stderr, l);
            fprintf(stderr, " is required\n");
            some_was_not_set = 1;
        }
    }
    if (some_was_not_set) return 1;

    return 0;
}
#endif
