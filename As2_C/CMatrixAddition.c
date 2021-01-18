// Includes
#include <stdio.h> // Printing output
#include <stdlib.h> // For rand() and srand()
#include <time.h> // srand() seed
#include <Windows.h> // For elapsed time
// Custom headers
#include "MatrixFunctions.h"
#include "CMatrixAddition.h"

#define MATRIX_SIZE 100 // Uses 100x100 matrices. Can be changed to make the matrices NxN


void Add_Matrix(int** matrix1, int** matrix2) {
    /* Sources:
    * "Acquiring high-resolution time stamps" - https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps#using-qpc-in-native-code
    */
    // Initialize variables
    int i, j;
    LARGE_INTEGER start, end, elapsed;
    LARGE_INTEGER freq;

    // Start measurint elapsed time
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);


    // Perform the element-wise addition of the two matrices, save result to matrix1
    for (i = 0; i < MATRIX_SIZE; i++) {
        for (j = 0; j < MATRIX_SIZE; j++) {
            matrix1[i][j] = matrix1[i][j] + matrix2[i][j];
        }
    }
    // Stop measuring, calculation is complete
    QueryPerformanceCounter(&end);
    elapsed.QuadPart = end.QuadPart - start.QuadPart;

    elapsed.QuadPart *= 1000000;
    elapsed.QuadPart /= freq.QuadPart;

    // Print the resulting matrix to make sure the program works correctly
    //Print_Matrix(matrix1, MATRIX_SIZE);

    // Output the time it took to perform the element-wise addition
    printf("Matrix calculation took %ld microseconds", elapsed);
}

int main(void) {
    // Define variables
    int** matrix1;
    int** matrix2;

    // Current time as random number seed
    srand(time(0));

    // Create two random matrices which will be added
    matrix1 = Create_Matrix(MATRIX_SIZE);
    matrix2 = Create_Matrix(MATRIX_SIZE);

    // Print the matrices
    //Print_Matrix(matrix1, MATRIX_SIZE);
    //Print_Matrix(matrix2, MATRIX_SIZE);

    // Perform matrix addition with the two previously created matrices
    Add_Matrix(matrix1, matrix2);

    // Free the memory allocated to the matrices
    Free_Matrix(matrix1, MATRIX_SIZE);
    Free_Matrix(matrix2, MATRIX_SIZE);
    printf("\n\n");

    // Program is done
    return 0;
}