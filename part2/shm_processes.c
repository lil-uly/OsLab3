#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

#define BANK_ACCOUNT_INDEX 0
#define TURN_INDEX 1

void ParentProcess(int[]);
void ChildProcess(int[]);

int main(int argc, char *argv[]) {
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    if (argc != 5) {
        printf("Use: %s #1 #2 #3 #4\n", argv[0]);
        exit(1);
    }

    ShmID = shmget(IPC_PRIVATE, 4 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error (server) ***\n");
        exit(1);
    }

    ShmPTR = (int *)shmat(ShmID, NULL, 0);
    if (ShmPTR == (int *)(-1)) {
        printf("*** shmat error (server) ***\n");
        exit(1);
    }

    ShmPTR[BANK_ACCOUNT_INDEX] = atoi(argv[1]);
    ShmPTR[TURN_INDEX] = 0;

    printf("Server has received a shared memory of four integers...\n");
    printf("Server has filled %d %d %d %d in shared memory...\n",
           ShmPTR[0], ShmPTR[1], ShmPTR[2], ShmPTR[3]);

    printf("Server is about to fork a child process...\n");
    pid = fork();
    if (pid < 0) {
        printf("*** fork error (server) ***\n");
        exit(1);
    } else if (pid == 0) {
        ChildProcess(ShmPTR);
        exit(0);
    } else {
        ParentProcess(ShmPTR);
        wait(&status);
        printf("Server has detected the completion of its child...\n");
        shmdt((void *)ShmPTR);
        shmctl(ShmID, IPC_RMID, NULL);
        printf("Server exits...\n");
        exit(0);
    }
}

void ParentProcess(int SharedMem[]) {
    srand((unsigned int)getpid()); // Seed for random number generation
    int account, turn;

    for (int i = 0; i < 25; ++i) {
        sleep(rand() % 6); // Sleep for random time between 0 - 5 seconds

        do {
            turn = SharedMem[TURN_INDEX];
        } while (turn != 0); // Wait for child's turn

        account = SharedMem[BANK_ACCOUNT_INDEX];

        if (account <= 100) {
            int balance = rand() % 101; // Random balance to deposit
            if (balance % 2 == 0) {
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }

        SharedMem[BANK_ACCOUNT_INDEX] = account;
        SharedMem[TURN_INDEX] = 1; // Set turn for child
    }
}

void ChildProcess(int SharedMem[]) {
    srand((unsigned int)getpid()); // Seed for random number generation
    int account, turn;

    for (int i = 0; i < 25; ++i) {
        sleep(rand() % 6); // Sleep for random time between 0 - 5 seconds

        do {
            turn = SharedMem[TURN_INDEX];
        } while (turn != 1); // Wait for parent's turn

        account = SharedMem[BANK_ACCOUNT_INDEX];

        int balance = rand() % 51; // Random balance needed
        printf("Poor Student needs $%d\n", balance);

        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }

        SharedMem[BANK_ACCOUNT_INDEX] = account;
        SharedMem[TURN_INDEX] = 0; // Set turn for parent
    }
}
