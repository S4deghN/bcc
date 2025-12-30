#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <sys/wait.h>

#define DA_INIT_CAP 256

#define reserve(list, minimum_cap) do {                                             \
    if ((list)->capacity == 0) {                                                    \
        (list)->capacity = DA_INIT_CAP;                                             \
    }                                                                               \
    while ((minimum_cap) > (list)->capacity) {                                      \
        (list)->capacity *= 2;                                                      \
    }                                                                               \
    (list)->data = realloc((list)->data, (list)->capacity * sizeof(*(list)->data)); \
    assert((list)->data != NULL);                                                   \
} while(0);

#define append_many(list, items, n) do {                                    \
    reserve((list), (list)->count + n);                                     \
    memcpy((list)->data + (list)->count, items, n * sizeof(*(list)->data)); \
    (list)->count += n;                                                     \
} while(0);

#define append(list, item) do {           \
    reserve((list), (list)->count + 1);   \
    (list)->data[(list)->count++] = item; \
} while(0);

typedef struct {
    char *data;
    size_t count;
    size_t capacity;
} Buff;

// TODO: Maybe use libc's `popen` and friends for portability.
int run_cmd(char *argv[], Buff *out_buff) {
    int pipefd[2];
    assert(pipe(pipefd) != -1);

    pid_t cpid = fork();
    if (cpid == 0) {
        close(pipefd[0]); // close unused read end.
        dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to pipe.
        if (execvp(argv[0], argv)) {
            perror("fork: ");
            exit(1);
        }
    }
    assert(cpid != -1);

    close(pipefd[1]); // close unused write end.

    int n;
    char buff[1024];
    while((n = read(pipefd[0], buff, sizeof buff))) {
        append_many(out_buff, buff, n)
    }

    int wstatus;
    if (waitpid(cpid, &wstatus, 0) == -1) {
        // TODO: handle based on how we exited. read `waitpid(2)`.
        return -1;
    }
    return 0;
}

#endif
