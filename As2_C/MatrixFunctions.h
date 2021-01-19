#ifndef MATRIXFUNCTIONS_H_INCLUDED
#define MATRIXFUNCTIONS_H_INCLUDED
// Standard include guard

/*********************************************************
* THIS IS THE HEADER FOR COMMON MATRIX RELATED FUNCTIONS
*********************************************************/

/**
* \brief Creates a NxN matrix populated with random numbers. Value of N is based on the variable size
* \param matrix_size Defines the matrix's size which will be sizeXsize
* \return A NxN matrix or NULL if not able to create the matrix
*/
int* createMatrix(int matrix_size);

/**
* \brief Prints a given matrix
* \param matrix Matrix to print
* \param matrix_size Row/Column length of the NxN matrix
* \return Nothing
*/
void printMatrix(int* matrix, int matrix_size);

/**
* \brief Frees the memory assigned to a matrix when it was created with Create_Matrix()
* \param matrix Matrix to free from memory
* \return Nothing
*/
void freeMatrix(int* matrix);

// End include guard
#endif