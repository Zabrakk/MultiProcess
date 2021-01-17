// Includes
#include <stdio.h> // Printing output
#include <stdlib.h> // For rand() and srand()
#include <time.h> // srand() seed
#include <sys/timeb.h> // For elapsed time
// Custom headers
#include "CMatrixAddition.h"


#define MATRIX_SIZE 100 // Uses 100x100 matrices. Can be changed to make the matrices NxN

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
        return NULL;
    }
    // Add MATRIX_SIZE rows
    for (i = 0; i < MATRIX_SIZE; i++) {
        matrix[i] = malloc(MATRIX_SIZE * sizeof(int));
        // Again, make sure that the memory was actually allocated
        if (matrix[i] == NULL) {
            printf("Could not allocate memory for the matrix");
            return NULL;
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
    /* Sources:
    * "How to get the time elapsed in C in milliseconds? (Windows)" - https://stackoverflow.com/questions/17250932/how-to-get-the-time-elapsed-in-c-in-milliseconds-windows/17252934
    */
    // Initialize variables
    int i, j, elapsed;
    struct timeb start, end;
    // Start measurint elapsed time
    ftime(&start);
    // Perform the element-wise addition of the two matrices, save result to matrix1
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix1[i][j] = matrix1[i][j] + matrix2[i][j];
        }
    }
    // Stop measuring, calculation is complete
    ftime(&end);
    elapsed = (int)(1000.0 * (end.time - start.time) + (end.millitm - start.millitm));

    // Print the resulting matrix to make sure the program works correctly
    //Print_Matrix(matrix1);

    // Output the time it took to perform the element-wise addition
    printf("Matrix calculation took %u milliseconds", elapsed);
}

void Print_Matrix(int** matrix) {
    //Initialize variables
    int i, j;
    // Loop over matrix columns and rows to print elements one by one
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

void Free_Matrix(int** matrix) {
    /* Sources:
    * "How to free 2d array in C?" - https://stackoverflow.com/questions/5666214/how-to-free-2d-array-in-c
    */
    for (int i = 0; i < MATRIX_SIZE; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

int main(void) {
    // Define variables
    int** matrix1;
    int** matrix2;
    // Current time as random number seed
    srand(time(0));
    // Create two random matrices which will be added
    matrix1 = Create_Matrix();
    matrix2 = Create_Matrix();

    // Print the matrices
    //Print_Matrix(matrix1);
    //Print_Matrix(matrix2);

    Add_Matrix(matrix1, matrix2);

    // Free the memory allocated to the matrices
    Free_Matrix(matrix1);
    Free_Matrix(matrix2);

    // Program is done
    return 0;
}


