#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

void ParentProcess(int []);
void ChildProcess(int []);

int main() {
    int ShmID;
    int *ShmPTR;
    pid_t pid;
    int status;

    ShmID = shmget(IPC_PRIVATE, 2 * sizeof(int), IPC_CREAT | 0666);
    if (ShmID < 0) {
        printf("*** shmget error ***\n");
        exit(1);
    }

    ShmPTR = (int *) shmat(ShmID, NULL, 0);
    if (*ShmPTR == -1) {
        printf("*** shmat error ***\n");
        exit(1);
    }

    // Initialize BankAccount and Turn
    ShmPTR[0] = 0; // BankAccount
    ShmPTR[1] = 0; // Turn (0 for Parent, 1 for Child)

    pid = fork();
    if (pid < 0) {
        printf("*** fork error ***\n");
        exit(1);
    } else if (pid == 0) {
        ChildProcess(ShmPTR);
        exit(0);
    } else {
        ParentProcess(ShmPTR);
    }

    wait(&status);
    shmdt((void *) ShmPTR);
    shmctl(ShmID, IPC_RMID, NULL);
    return 0;
}

void ParentProcess(int SharedMem[]) {
    srand(time(NULL));
    for (int i = 0; i < 25; i++) {
        sleep(rand() % 6);  // Sleep between 0-5 seconds
        int account = SharedMem[0];  // BankAccount
        while (SharedMem[1] != 0);  // Wait for Turn to be 0

        if (account <= 100) {
            int balance = rand() % 101;  // Random between 0 and 100
            if (balance % 2 == 0) {
                account += balance;
                printf("Dear old Dad: Deposits $%d / Balance = $%d\n", balance, account);
            } else {
                printf("Dear old Dad: Doesn't have any money to give\n");
            }
        } else {
            printf("Dear old Dad: Thinks Student has enough Cash ($%d)\n", account);
        }
        SharedMem[0] = account;
        SharedMem[1] = 1;  // Set Turn to 1 (Child's turn)
    }
}

void ChildProcess(int SharedMem[]) {
    srand(time(NULL) ^ getpid());  // Different seed for child
    for (int i = 0; i < 25; i++) {
        sleep(rand() % 6);  // Sleep between 0-5 seconds
        int account = SharedMem[0];  // BankAccount
        while (SharedMem[1] != 1);  // Wait for Turn to be 1

        int balance = rand() % 51;  // Random between 0 and 50
        printf("Poor Student needs $%d\n", balance);
        if (balance <= account) {
            account -= balance;
            printf("Poor Student: Withdraws $%d / Balance = $%d\n", balance, account);
        } else {
            printf("Poor Student: Not Enough Cash ($%d)\n", account);
        }
        SharedMem[0] = account;
        SharedMem[1] = 0;  // Set Turn to 0 (Parent's turn)
    }
}
