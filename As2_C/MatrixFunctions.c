#include "MatrixFunctions.h"
#include <stdlib.h>


int** Create_Matrix(int matrix_size) {
    /* Sources:
    * "How To Dynamically Allocate a 2D Array in C" - https://www.youtube.com/watch?v=t72BzxMAQKs
    * "Generating random number in a range in C" - https://www.geeksforgeeks.org/generating-random-number-range-c/
    */
    // Define variables
    int** matrix;
    int i, j;
    int min = 0, max = 20; // Random matrix values are between 0 and 20

    // Make sure that the given matrix size is more than 0
    if (matrix_size < 1) {
        printf("Matrix can't have 0 elements!\n");
        return NULL;
    }

    // Initialize memory for MATRIX_SIZE columns
    matrix = malloc(matrix_size * sizeof(int*));
    // Make sure that the memory was actually allocated
    if (matrix == NULL) {
        printf("Could not allocate memory for the matrix");
        return NULL;
    }

    // Add MATRIX_SIZE rows
    for (i = 0; i < matrix_size; i++) {
        matrix[i] = malloc(matrix_size * sizeof(int));
        // Again, make sure that the memory was actually allocated
        if (matrix[i] == NULL) {
            printf("Could not allocate memory for the matrix");
            return NULL;
        }
    }

    // Populate the matrix with random numbers
    for (i = 0; i < matrix_size; i++) {
        for (j = 0; j < matrix_size; j++) {
            matrix[i][j] = (rand() % (max - min + 1)) + min;
        }
    }
    return matrix;
}

void Print_Matrix(int** matrix, int matrix_size) {
    //Initialize variables
    int i, j;

    // Loop over matrix columns and rows to print elements one by one
    for (i = 0; i < matrix_size; i++) {
        printf("[");
        for (j = 0; j < matrix_size; j++) {
            if (j < matrix_size - 1) {
                printf("%d, ", matrix[i][j]);
            }
            else {
                printf("%d]\n", matrix[i][j]);
            }
        }
    }
    printf("\n");
}

void Free_Matrix(int** matrix, int matrix_size) {
    /* Sources:
    * "How to free 2d array in C?" - https://stackoverflow.com/questions/5666214/how-to-free-2d-array-in-c
    */
    for (int i = 0; i < matrix_size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}