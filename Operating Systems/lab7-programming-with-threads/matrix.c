#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX 20
#define NUM_THREADS 10

int matA[MAX][MAX]; 
int matB[MAX][MAX]; 

int matSumResult[MAX][MAX];
int matDiffResult[MAX][MAX]; 
int matProductResult[MAX][MAX]; 

// Structure to hold arguments for each thread
typedef struct {
    int start_row;
    int end_row;
} ThreadArgs;

// Function to fill a matrix with random values
void fillMatrix(int matrix[MAX][MAX]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            matrix[i][j] = rand() % 10 + 1;
        }
    }
}

// Function to print a matrix
void printMatrix(int matrix[MAX][MAX]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Compute matrix addition for each assigned row range
void* computeSum(void* args) {
    ThreadArgs* range = (ThreadArgs*) args;
    for (int i = range->start_row; i < range->end_row; i++) {
        for (int j = 0; j < MAX; j++) {
            matSumResult[i][j] = matA[i][j] + matB[i][j];
        }
    }
    return NULL;
}

// Compute matrix subtraction for each assigned row range
void* computeDiff(void* args) {
    ThreadArgs* range = (ThreadArgs*) args;
    for (int i = range->start_row; i < range->end_row; i++) {
        for (int j = 0; j < MAX; j++) {
            matDiffResult[i][j] = matA[i][j] - matB[i][j];
        }
    }
    return NULL;
}

// Compute matrix dot product for each assigned row range
void* computeProduct(void* args) {
    ThreadArgs* range = (ThreadArgs*) args;
    for (int i = range->start_row; i < range->end_row; i++) {
        for (int j = 0; j < MAX; j++) {
            matProductResult[i][j] = 0;
            for (int k = 0; k < MAX; k++) {
                matProductResult[i][j] += matA[i][k] * matB[k][j];
            }
        }
    }
    return NULL;
}

int main() {
    srand(time(0));

    // 1. Fill the matrices (matA and matB) with random values
    fillMatrix(matA);
    fillMatrix(matB);

    // 2. Print the initial matrices
    printf("Matrix A:\n");
    printMatrix(matA);
    printf("Matrix B:\n");
    printMatrix(matB);

    // 3. Create pthread_t objects for our threads
    pthread_t threads[NUM_THREADS];
    ThreadArgs thread_args[NUM_THREADS];

    int rows_per_thread = MAX / NUM_THREADS;

    // 4. Create threads for sum operation
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].start_row = i * rows_per_thread;
        thread_args[i].end_row = (i == NUM_THREADS - 1) ? MAX : (i + 1) * rows_per_thread;
        pthread_create(&threads[i], NULL, computeSum, (void*) &thread_args[i]);
    }

    // 5. Wait for sum threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Repeat steps 4 and 5 for diff and product operations
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, computeDiff, (void*) &thread_args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, computeProduct, (void*) &thread_args[i]);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    // 6. Print the results
    printf("Sum:\n");
    printMatrix(matSumResult);
    printf("Difference:\n");
    printMatrix(matDiffResult);
    printf("Product:\n");
    printMatrix(matProductResult);

    return 0;
}
