#ifndef CMATRIXADDITION_H_INCLUDED
#define CMATRIXADDITION_H_INCLUDED
// Standard include guard

#include <stdio.h>

/**
* \brief Performs element-wise addition for the two given matrices. Prints the elapsed time
* \param matrix1 100x100 Matrix created with malloc
* \param matrix2 100x100 Matrix created with malloc
* \return Nothing
*/
void Add_Matrix(int** matrix1, int** matrix2);

/**
* \brief Creates a 100x100 matrix populated with random numbers
* \return A 2D pointer 100x100 matrix or NULL if not able to create the matrix
*/
int** Create_Matrix();

/**
* \brief Prints a given matrix
* \return Nothing
*/
void Print_Matrix(int** matrix);

// End include guard
#endif