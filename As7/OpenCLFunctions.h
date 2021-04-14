#ifndef OPENCLFUNCTIONS_H_INCLUDED
#define OPENCLFUNCTIONS_H_INCLUDED
// Standard include guard

/*********************************************************
* THIS IS THE HEADER FOR COMMON OPENCL RELATED FUNCTIONS
*********************************************************/

#include <vector>
// OpenCL include
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif // __APPLE__

/*
* \brief Struct that holds the kernel function and its size
* \param source_str Contents of the Kernel. Must bee freed at the end of the program
* \param source_size Size of the Kernel
*/
typedef struct {
	char* source_str;
	size_t source_size;
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
* \return 1 if succesfully loaded; 0 otherwise
*/
int loadKernel(char file_name[], kernel_source *src);

/*
* \brief Creates a Kernel from the given function name and source
* \param context OpenCL context
* \param device_id Device to use
* \param kernel_name Name of the function
* \param src Kernel source code
* \param size Size of the Kernel source code
* \return OpenCL Kernel or NULL if failed to create
*/
cl_kernel createKernel(cl_context context, cl_device_id device_id, char* kernel_name, const char** src, const size_t* size);

/*
* \brief Selects the OpenCL device with the name "NVIDIA GeForce GTX 1070", prints its info and returns the deivce
* \return GPU device id
*/
cl_device_id getGPUDevice();

/*
* \brief Prints information about the given OpenCL device
* \param device cl_device_id of the wanted device
* \return Nothing
*/
void printDeviceInfo(cl_device_id device);

/*
* \brief Returns the OpenCL 2D image format used for RGBA images
* \return RGBA 2D image format
*/
cl_image_format getRGBAImageFormat();

/*
* \brief Returns the OpenCL 2D image format used for grayscale images
* \return Grayscale 2D image format
*/
cl_image_format getGrayImageFormat();

/*
* \brief Runs the resize + grayscale kernel, given as parameter
* \param cmd_q OpenCL command queue
* \param kernel OpenCL kernel for resize + grayscale
* \param new_w Image width after resize
* \param new_h Image height after resize
* \param out_cl OpenCL mem object for the output image
* \return The resulting image
*/
std::vector<unsigned char> executeImageKernel(cl_command_queue cmd_q, cl_kernel kernel, unsigned new_w, unsigned new_h, cl_mem out_cl);


std::vector<unsigned char> executeBufferKernel(cl_command_queue cmd_q, cl_kernel kernel, size_t global_size[], size_t local_size[], unsigned new_w, unsigned new_h, cl_mem out_cl);

#endif