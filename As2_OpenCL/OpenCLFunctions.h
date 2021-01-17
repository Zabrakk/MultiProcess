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
* \brief Check for OpenCL errors based on given error number
* \param err_num Error number from OpenCL functions
* \return 1 if no errors; 0 otherwise
*/
int errorCheck(cl_int err_num);

/*
* \brief Reads a kernel function from a given function
* \param fileName Name of the kernel file
* \return A kernel_source struct containing the kernel's contents and size. ok = 0 if failed to create
*/
kernel_source loadKernel(char file_name[]);

/*
* \brief Obtains the first OpenCL GPU device id
* \return cl_device_id or NULL if failed to find a device
*/
cl_device_id getGPUDevice();

/*
* \brief Creates an OpenCL context for the given device
* \param device_id ID of the device
* \return OpenCL context or NULL if failed to create context
*/
cl_context getContext(cl_device_id device_id);

/*
* \brief Creates an OpenCL Command Queue for the specified context and device
* \param context OpenCL context
* \param device_id OpenCL device ID
* \return Command queue or NULL if failed to create
*/
cl_command_queue getCommandQueue(cl_context context, cl_device_id device_id);

// End include guard
#endif