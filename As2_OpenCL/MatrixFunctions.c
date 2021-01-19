#include "MatrixFunctions.h"
#include <stdlib.h>
#include <stdio.h>

int* createMatrix(int matrix_size) {
    /* Sources:
    * "Malloc a 2D array in C [duplicate]" - https://stackoverflow.com/questions/36890624/malloc-a-2d-array-in-c
    * "Generating random number in a range in C" - https://www.geeksforgeeks.org/generating-random-number-range-c/
    */
    int* matrix;
    int i, j;
    int min = 0;
    int max = 20;

    // Allocate memory for the matrix
    matrix = malloc(matrix_size * matrix_size * sizeof(int));
    if (matrix == NULL) return NULL;
    // Populate with numbers
    for (i = 0; i < matrix_size; i++) {
        for (j = 0; j < matrix_size; j++) {
            // Generate random numbers in the range of 0-20
            matrix[i * matrix_size + j] = (rand() % (max - min + 1)) + min;
        }
    }
    return matrix;
}

void printMatrix(int* matrix, int matrix_size) {
    int i, j;

    // Loop over matrix columns and rows to print elements one by one
    for (i = 0; i < matrix_size; i++) {
        printf("[");
        for (j = 0; j < matrix_size; j++) {
            if (j < matrix_size - 1) {
                printf("%d, ", matrix[i * matrix_size + j]);
            }
            else {
                printf("%d]\n", matrix[i * matrix_size + j]);
            }
        }
    }
    printf("\n");
}

void freeMatrix(int* matrix) {
    free(matrix);
}