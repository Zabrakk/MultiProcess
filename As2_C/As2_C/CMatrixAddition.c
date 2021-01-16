// Includes
#include <stdio.h> // Printing output
#include <stdlib.h> // For rand() and srand()
#include <time.h> // For elapsed time, and srand() seed
// Custom headers
#include "CMatrixAddition.h"

// FIX TIMER! ! ! !  !

#define MATRIX_SIZE 900 // Uses 100x100 matrices

int** Create_Matrix() {
    /* Sources:
    * "How To Dynamically Allocate a 2D Array in C" - https://www.youtube.com/watch?v=t72BzxMAQKs
    * "Generating random number in a range in C" - https://www.geeksforgeeks.org/generating-random-number-range-c/
    */
    // Define variables
    int** matrix;
    int i, j;
    int min = 0, max = 20;
    // Initialize memory for MATRIX_SIZE columns
    matrix = malloc(MATRIX_SIZE * sizeof(int*));
    // Make sure that the memory was actually allocated
    if (matrix == NULL) {
        printf("Could not allocate memory for the matrix");
        getchar();
        exit(1);
    }
    // Add MATRIX_SIZE rows
    for (i = 0; i < MATRIX_SIZE; i++) {
        matrix[i] = malloc(MATRIX_SIZE * sizeof(int));
        // Again, make sure that the memory was actually allocated
        if (matrix[i] == NULL) {
            printf("Could not allocate memory for the matrix");
            getchar();
            exit(1);
        }
    }
 
    // Populate the matrix with random numbers
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix[i][j] = (rand() % (max - min + 1)) + min;
        }
    }
    return matrix;
}

void Add_Matrix(int** matrix1, int** matrix2) {
    // Initialize variables
    int i, j;
    clock_t t;
    long elapsed;
    // Start timer
    t = clock();
    // Perform the element-wise addition of the two matrices, save result to matrix1
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix1[i][j] = matrix1[i][j] + matrix2[i][j];
        }
    }
    // Get time elapsed during the addition
    t = clock() - t;
    elapsed = ((double)t) / CLOCKS_PER_SEC * 1000;

    // Print the resulting matrix
    //Print_Matrix(matrix1);

    printf("Matrix calculation took %ld ms", elapsed);
}

void Print_Matrix(int** matrix) {
    //Initialize variables
    int i, j;
    for (i = 0; i < MATRIX_SIZE; i++) {
        printf("[");
        for (j = 0; j < MATRIX_SIZE; j++) {
            if (j < MATRIX_SIZE-1) {
                printf("%d, ", matrix[i][j]);
            }
            else {
                printf("%d]\n", matrix[i][j]);
            }
        }
    }
    printf("\n");
}

int main(void) {
    int** matrix1, matrix2;
    // Current time as random number seed
    srand(time(0));
    matrix1 = Create_Matrix();
    matrix2 = Create_Matrix();

    //Print_Matrix(matrix1);
    //Print_Matrix(matrix2);

    Add_Matrix(matrix1, matrix2);

    // REMEMBER TO FREE THE MATRICES
    free(matrix1);
    free(matrix2);

    // Program is done
    return 0;
}


