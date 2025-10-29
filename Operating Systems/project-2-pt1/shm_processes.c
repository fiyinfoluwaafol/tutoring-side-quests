#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/stat.h>

// Updated function to include student ID
void poor_student(sem_t *mutex, int *bankAccount, int studentID) {
    while (1) {
        sleep(rand() % 5); 
        printf("Poor Student #%d: Attempting to Check Balance\n", studentID);

        sem_wait(mutex); 
        int localBalance = *bankAccount;
        int randomNum = rand();
        if (randomNum % 2 == 0) {
            int need = rand() % 50; 
            printf("Poor Student #%d needs $%d\n", studentID, need);
            if (need <= localBalance) {
                localBalance -= need;
                printf("Poor Student #%d: Withdraws $%d / Balance = $%d\n", studentID, need, localBalance);
            } else {
                printf("Poor Student #%d: Not Enough Cash ($%d)\n", studentID, localBalance);
            }
            *bankAccount = localBalance; 
        } else {
            printf("Poor Student #%d: Last Checking Balance = $%d\n", studentID, localBalance);
        }
        sem_post(mutex); 
    }
}

void dear_old_dad(sem_t *mutex, int *bankAccount) {
    while (1) {
        sleep(rand() % 5);
        printf("Dear Old Dad: Attempting to Check Balance\n");

        sem_wait(mutex); 
        int localBalance = *bankAccount;
        int randomNum = rand();
        if (randomNum % 2 == 0) {
            if (localBalance < 100) {
                int amount = rand() % 100; 
                if (rand() % 2 == 0) {
                    localBalance += amount;
                    printf("Dear Old Dad: Deposits $%d / Balance = $%d\n", amount, localBalance);
                } else {
                    printf("Dear Old Dad: Doesn't have any money to give\n");
                }
                *bankAccount = localBalance;
            } else {
                printf("Dear Old Dad: Thinks Student has enough Cash ($%d)\n", localBalance);
            }
        } else {
            printf("Dear Old Dad: Last Checking Balance = $%d\n", localBalance);
        }
        sem_post(mutex); 
    }
}

void lovable_mom(sem_t *mutex, int *bankAccount) {
    while (1) {
        sleep(rand() % 10); 
        printf("Lovable Mom: Attempting to Check Balance\n");

        sem_wait(mutex); 
        int localBalance = *bankAccount;
        if (localBalance <= 100) {
            int amount = rand() % 125; 
            localBalance += amount;
            printf("Lovable Mom: Deposits $%d / Balance = $%d\n", amount, localBalance);
            *bankAccount = localBalance;
        } else {
            printf("Lovable Mom: Thinks Student has enough Cash ($%d)\n", localBalance);
        }
        sem_post(mutex); 
    }
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num_parents> <num_children>\n", argv[0]);
        exit(1);
    }

    int num_parents = atoi(argv[1]);
    int num_children = atoi(argv[2]);

    int fd, zero = 0, *bankAccount;
    sem_t *mutex;

    // Shared memory for bank account
    fd = open("bank_account.txt", O_RDWR | O_CREAT, S_IRWXU);
    write(fd, &zero, sizeof(int)); 
    bankAccount = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    // Semaphore initialization
    if ((mutex = sem_open("bank_semaphore", O_CREAT, 0644, 1)) == SEM_FAILED) {
        perror("Semaphore initialization failed");
        exit(1);
    }

    srand(time(NULL)); // Seed for random number generation

    // Fork parent processes for "Dear Old Dad" and "Lovable Mom"
    for (int i = 0; i < num_parents; i++) {
        if (fork() == 0) {
            if (i == 0) {
                dear_old_dad(mutex, bankAccount);
            } else {
                lovable_mom(mutex, bankAccount);
            }
            exit(0);
        }
    }

    // Fork child processes for "Poor Students"
    for (int i = 0; i < num_children; i++) {
        if (fork() == 0) {
            poor_student(mutex, bankAccount, i + 1); // Pass unique student ID
            exit(0);
        }
    }

    // Keep the main process alive
    while (1) {
        sleep(1);
    }

    return 0;
}