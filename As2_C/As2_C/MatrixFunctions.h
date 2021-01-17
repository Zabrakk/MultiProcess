#ifndef MATRIXFUNCTIONS_H_INCLUDED
#define MATRIXFUNCTIONS_H_INCLUDED
// Standard include guard

/*********************************************************
* THIS IS THE HEADER FOR COMMON MATRIX RELATED FUNCTIONS
*********************************************************/

/**
* \brief Creates a NxN matrix populated with random numbers. Value of N is based on the variable size
* \param matrix_size Defines the matrix's size which will be sizeXsize
* \return A 2D pointer NxN matrix or NULL if not able to create the matrix
*/
int** Create_Matrix(int matrix_size);

/**
* \brief Prints a given matrix
* \param matrix_size The value of N in NxN matrix
* \return Nothing
*/
void Print_Matrix(int** matrix, int matrix_size);

/**
* \brief Frees the memory assigned to a matrix when it was created with Create_Matrix()
* \param matrix_size The value of N in NxN matrix
* \return Nothing
*/
void Free_Matrix(int** matrix, int matrix_size);

// End include guard
#endif