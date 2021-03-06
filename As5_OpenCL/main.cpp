#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <Windows.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include "lodepng.h"
#include "ImageFunctions.h"
#include "OpenCLFunctions.h"

#define WINDOW_Y 13 
#define WINDOW_X 11 
#define MIN_DISPARITY 0
#define MAX_DISPARITY 65 // Scaled down. 260/4 as stated in the Assignment
#define THRESHOLD 3

#define KERNEL_FINE_NAME "kernel.cl" // Kernel file name

int main() {
	// Image width and height. Both input images are the same size
	unsigned w = 2940;
	unsigned h = 2016;
	// Timing info
	timer_struct timer;

	/**********************************************/
	/*  Load images with the original C++ code    */
	/**********************************************/
	/*
	// Initialize original images
	unsigned char* im0 = NULL;
	unsigned char* im1 = NULL;

	// Read the original images
	if (ReadImage(&im0, "im0.png", w, h)) return 1;
	if (ReadImage(&im1, "im1.png", w, h)) return 1;
	// Convert to a nicer format, i.e. vector
	std::vector<unsigned char> im0_vector = charArrToVector(im0, w * h * 4);
	std::vector<unsigned char> im1_vector = charArrToVector(im1, w * h * 4);
	// Free the unsigned char arrays from memory
	delete im0;
	delete im1;
	*/
	/**********************************************/
	/*				  OPENCL                      */
	/**********************************************/

	cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue cmd_q = NULL;
	cl_event event = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	cl_int err_num = NULL;
	cl_uint num_of_platforms;
	cl_uint num_of_devices;

	// Load Kernel source into memory
	//kernel_source src = loadKernel(KERNEL_FINE_NAME);
	//if (src.ok == 0) return 1;

	// Select the first platform
	if (!errorCheck(clGetPlatformIDs(1, &platform_id, &num_of_platforms))) return 1;
	// Obtain first device from the selected platform with type GPU
	if (!errorCheck(clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_of_devices))) return 1;
	// Print info about the selected device
	printDeviceInfo(device_id);


	printf("Done! Enter something to exit: ");
	getchar();
	return 0;
}