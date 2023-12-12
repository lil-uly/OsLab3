#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int fd1[2];  // Used to store two ends of first pipe
    int fd2[2];  // Used to store two ends of second pipe

    char fixed_str_p1[] = "howard.edu";
    char fixed_str_p2[] = "gobison.org";
    char input_str[100];
    pid_t p;

    if (pipe(fd1) == -1 || pipe(fd2) == -1) {
        fprintf(stderr, "Pipe Failed");
        return 1;
    }

    p = fork();

    if (p < 0) {
        fprintf(stderr, "fork Failed");
        return 1;
    } else if (p > 0) { // Parent process
        close(fd1[0]);  // Close reading end of first pipe
        close(fd2[1]);  // Close writing end of second pipe

        printf("Enter a string to concatenate: ");
        scanf("%s", input_str);

        // Write input string and close writing end of first pipe.
        write(fd1[1], input_str, strlen(input_str) + 1);

        char concatenated_str[200];
        read(fd2[0], concatenated_str, sizeof(concatenated_str));

        printf("Final concatenated string: %s\n", concatenated_str);

        close(fd1[1]); // Close writing end of pipes
        close(fd2[0]);
        
        wait(NULL); // Wait for child process to finish
    } else { // Child process
        close(fd1[1]);  // Close writing end of first pipe
        close(fd2[0]);  // Close reading end of second pipe

        char received_str[100];
        read(fd1[0], received_str, sizeof(received_str));

        strcat(received_str, fixed_str_p1);
        printf("Received string in Child: %s\n", received_str);

        printf("Enter a string: ");
        scanf("%s", input_str);

        strcat(input_str, fixed_str_p2);
        printf("Child concatenated string: %s\n", input_str);

        write(fd2[1], input_str, strlen(input_str) + 1);

        close(fd1[0]);
        close(fd2[1]);

        exit(0);
    }
    return 0;
}
