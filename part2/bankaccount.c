#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>

#define MAX_LOOP 25

struct SharedMemory {
    int BankAccount;
    int Turn;
};

int main() {
    int shmid;
    struct SharedMemory *shmem;

    // Create shared memory segment
    shmid = shmget(IPC_PRIVATE, sizeof(struct SharedMemory), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    // Attach shared memory segment
    shmem = (struct SharedMemory *)shmat(shmid, NULL, 0);
    if ((void *)shmem == (void *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // Initialize shared variables
    shmem->BankAccount = 0;
    shmem->Turn = 0;

    // Semaphore initialization
    sem_t *mutex_parent, *mutex_child;
    mutex_parent = sem_open("/parent_semaphore", O_CREAT, 0644, 1);
    mutex_child = sem_open("/child_semaphore", O_CREAT, 0644, 0);
    if (mutex_parent == SEM_FAILED || mutex_child == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process (Poor Student)
        srand(getpid());

        for (int i = 0; i < MAX_LOOP; ++i) {
            usleep(rand() % 5000000); // Sleep random time up to 5 seconds

            sem_wait(mutex_child);
            int account = shmem->BankAccount;

            int balance = rand() % 51; // Random balance needed
            printf("Poor Student needs $%d\n", balance);

            if (balance <= account) {
                shmem->BankAccount -= balance;
                printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, shmem->BankAccount);
            } else {
                printf("Poor Student: Not Enough Cash ($%d)\n", account);
            }

            sem_post(mutex_parent);
        }
    } else {
        // Parent process (Dear Old Dad)
        srand(getpid());

        for (int i = 0; i < MAX_LOOP; ++i) {
            usleep(rand() % 5000000); // Sleep random time up to 5 seconds

            sem_wait(mutex_parent);
            int account = shmem->BankAccount;

            if (account <= 100) {
                int balance = rand() % 101; // Random balance to give
                if (balance % 2 == 0) {
                    shmem->BankAccount += balance;
                    printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, shmem->BankAccount);
                } else {
                    printf("Dear old Dad: Doesn't have any money to give\n");
                }
            } else {
                printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
            }

            sem_post(mutex_child);
        }

        // Wait for the child process to finish
        wait(NULL);

        // Detach and remove shared memory segment
        shmdt(shmem);
        shmctl(shmid, IPC_RMID, NULL);

        // Close and unlink semaphores
        sem_close(mutex_parent);
        sem_close(mutex_child);
        sem_unlink("/parent_semaphore");
        sem_unlink("/child_semaphore");
    }

    return 0;
}
