#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <errno.h>

void run_popen(int argc, char *argv[]) {
    char buff[254];
    char *p = buff;
    for (int i = 1; i < argc; ++i) {
        p = stpcpy(p, argv[i]);
        p = stpcpy(p, " ");
    }

    int n;
    FILE *f = popen(buff, "er");
    while(n = fread(buff, 1, sizeof(buff), f)) {
        fwrite(buff, 1, n, stdout);
    }

    int err = pclose(f);
    if (err == -1) {
        printf("error: %d\n", err);
    }
    exit(err);
}

void run_pipe(int argc, char *argv[]) {
    char buff[254];

    int pipefd[2];
    assert(pipe(pipefd) != -1);

    pid_t cpid = fork();
    if (cpid == 0) {
        close(pipefd[0]); // close unused read end.
        dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to pipe.
        if (execvp(argv[1], argv + 1)) {
            perror("fork:");
            exit(1);
        }
    } else {
        close(pipefd[1]); // close unused write end.

        int n;
        while(n = read(pipefd[0], buff, sizeof(buff))) {
            write(STDOUT_FILENO, buff, n);
        }

        int wstatus;
        if (waitpid(cpid, &wstatus, 0) == -1) {
            exit(1);
        }
        // TODO: handle everything!
        if (WIFEXITED(wstatus)) {
            printf("exited, status=%d\n", WEXITSTATUS(wstatus));
        } else if (WIFSIGNALED(wstatus)) {
            printf("killed by signal %d\n", WTERMSIG(wstatus));
        } else if (WIFSTOPPED(wstatus)) {
            printf("stopped by signal %d\n", WSTOPSIG(wstatus));
        } else if (WIFCONTINUED(wstatus)) {
            printf("continued\n");
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) return 1;
    // run_popen(argc, argv);
    run_pipe(argc, argv);
}

