// Includes
#include <stdio.h> // Printing output
#include <stdlib.h> // For rand() and srand()
#include <time.h> // srand() seed
#include <Windows.h> // For elapsed time
// Custom headers
#include "MatrixFunctions.h"

#define SIZE 100

int* Add_Matrix(int* m1, int* m2, int matrix_size) {
	/* Sources:
	* "Acquiring high-resolution time stamps" - https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
	*/
	int i, j;
	int* result = createMatrix(matrix_size);
	LARGE_INTEGER start, end, elapsed;
	LARGE_INTEGER freq;

	// Start measurint elapsed time with WINAPI
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	// Calculate the addition, store in a new matrix
	if (result == NULL) return NULL;
	for (i = 0; i < matrix_size; i++) {
		for (j = 0; j < matrix_size; j++) {
			result[i * matrix_size + j] = m1[i * matrix_size + j] + m2[i * matrix_size + j];
		}
	}

	// Stop measuring, calculation is complete
	QueryPerformanceCounter(&end);
	elapsed.QuadPart = end.QuadPart - start.QuadPart;
	elapsed.QuadPart *= 1000000;
	elapsed.QuadPart /= freq.QuadPart;

	// Output the time it took to perform the element-wise addition
	printf("Matrix calculation took %ld microseconds\n\n", elapsed);

	// Return the result
	return result;
}

int main() {
	// Current time as random number seed
	srand(time(0));

	// Create the matrices
	int* m1 = createMatrix(SIZE);
	int* m2 = createMatrix(SIZE);

	// Print the matrices
	//printMatrix(m1, SIZE);
	//printMatrix(m2, SIZE);

	// Calculate the addition
	int* result = Add_Matrix(m1, m2, SIZE);
	//printMatrix(result, SIZE);

	// Free the matrices from memory
	freeMatrix(m1);
	freeMatrix(m2);
	freeMatrix(result);

	return 0;
}