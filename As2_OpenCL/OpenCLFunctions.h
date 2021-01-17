#ifndef OPENCLFUNCTIONS_H_INCLUDED
#define OPENCLFUNCTIONS_H_INCLUDED
// Standard include guard

/*********************************************************
* THIS IS THE HEADER FOR COMMON OPENCL RELATED FUNCTIONS
*********************************************************/

// OpenCL include
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif // __APPLE__

/*
* \brief Struct that holds the kernel function and its size
* \param source_str Contents of the Kernel
* \param source_size Size of the Kernel
* \param ok 1 if initialized correctly; 0 otherwise
*/
typedef struct {
	char* source_str;
	size_t source_size;
	int ok;
} kernel_source;

/*
* \brief Reads a kernel function from a given function
* \param fileName Name of the kernel file
* \return A kernel_source struct containing the kernel's contents and size. ok = 0 if failed to create
*/
kernel_source LoadKernel(char file_name[]);

/*
* \brief Creates a GPU OpenCL context
* \return OpenCL context or NULL if failed to get context
*/
cl_context getGPUContext();


// End include guard
#endif