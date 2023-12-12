#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <argument>\n", argv[0]);
        return 1;
    }

    int pipefd[2];
    int pipefd2[2];
    int pid, pid2;

    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", argv[1], NULL};
    char *sort_args[] = {"sort", NULL};

    // Create pipes
    if (pipe(pipefd) == -1 || pipe(pipefd2) == -1) {
        perror("Pipe Failed");
        return 1;
    }

    pid = fork();

    if (pid == 0) {
    
        dup2(pipefd[1], 1);
        close(pipefd[0]);
        close(pipefd2[0]);
        close(pipefd2[1]);

        execvp("cat", cat_args);
    } else {
        pid2 = fork();
        if (pid2 == 0) {
  
            dup2(pipefd[0], 0);
            dup2(pipefd2[1], 1);
            close(pipefd[1]);
            close(pipefd2[0]);

            execvp("grep", grep_args);
        } else {

            dup2(pipefd2[0], 0);
            close(pipefd[0]);
            close(pipefd[1]);
            close(pipefd2[1]);

            execvp("sort", sort_args);

            wait(NULL);
            wait(NULL);
        }
    }

    return 0;
}
