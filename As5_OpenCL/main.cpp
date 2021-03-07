#include <stdio.h>
#include <vector>
#include <stdlib.h>
#include <Windows.h>
#include <math.h>

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

#define KERNEL_FILE_NAME "kernel.cl" // Kernel file name
#define KERNEL_RESIZE_GRAYSCALE "resize_and_grayscale"

int main() {
	// Image width and height. Both input images are the same size
	unsigned w = 2940;
	unsigned h = 2016;
	// Timing info
	timer_struct timer;

	/**********************************************/
	/*  Load images with the original C++ code    */
	/**********************************************/
	
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
	//delete im0;
	//delete im1;
	
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
	kernel_source src = loadKernel(KERNEL_FILE_NAME);
	if (src.ok == 0) return 1;

	// Select the first platform
	if (!errorCheck(clGetPlatformIDs(1, &platform_id, &num_of_platforms))) return 1;
	// Obtain first device from the selected platform with type GPU
	if (!errorCheck(clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_of_devices))) return 1;
	// Print info about the selected device
	printDeviceInfo(device_id);
	// Create the context
	printf("Creating context\n");
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	// Create command queue with profiling enabled, so Kernel exewcution time can be obtained
	printf("Creating command queue\n");
	cmd_q = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &err_num);
	if (!errorCheck(err_num)) return 1;

	// Create the resize & grayscale kernel
	printf("Creating Kernel for resizing and grayscaling\n");
	kernel = createKernel(context, device_id, KERNEL_RESIZE_GRAYSCALE, (const char**)&src.source_str, (const size_t*)&src.source_size);

	// Format for the OpenCL image objects
	cl_image_format format;
	format.image_channel_order = CL_RGBA; // The image was read as RGBA
	format.image_channel_data_type = CL_UNORM_INT8;

	// Create a 2D image object for im0 and im1
	printf("Creating 2D images objects for im0 and im1\n");
	cl_mem im0_cl = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &format, w, h, 0, &im0_vector[0], &err_num);
	if (!errorCheck(err_num)) return 1;
	//cl_mem im1_cl = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &format, w, h, 0, &im1_vector[0], &err_num);
	//if (!errorCheck(err_num)) return 1;

	// Create output format and object for the resized grayscaled image
	int new_w = floor(w / 4);
	int new_h = floor(h / 4);

	//cl_mem im0_grayscale = clCreateImage2D(context, CL_MEM_READ_WRITE, &format, new_w, new_h, 0, NULL, &err_num);
	cl_mem im0_grayscale = clCreateBuffer(context, CL_MEM_READ_WRITE, w * h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	//cl_mem im1_grayscale = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &format, new_w, new_h, 0, NULL, &err_num);
	//if (!errorCheck(err_num)) return 1;

	// Give parameters to Kernel
	printf("Passing parameters to the Kernel\n");
	err_num = clSetKernelArg(kernel, 0, sizeof(unsigned), &w);
	err_num |= clSetKernelArg(kernel, 1, sizeof(unsigned), &h);
	err_num |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &im0_cl);
	err_num |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &im0_grayscale);
	if (!errorCheck(err_num)) return 1;

	size_t global_work_size[] = { w, h }; // 32, 32 because Device Max Work Group Size is 1024
	size_t local_work_size[] = { 1, 1 }; // 32, 32 because Device Max Work Item Size is 1024 // IF YOU CHANGES THIS T 16, 16, e.g. THIS SHIT STOPS WORKING AT clEnqueueNDRangeKernel

	// Execute the Kernel
	printf("Executing the resize and grayscale Kernel\n");
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;

	// Wait for execution to finish
	clWaitForEvents(1, &event);
	cl_ulong opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	double milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);
	
	
	// Get the resulting grayscaled image
	std::vector<unsigned char> im0_gray_vector(w * h);
	err_num = clEnqueueReadBuffer(cmd_q, im0_grayscale, CL_TRUE, 0, w * h * sizeof(unsigned char), &im0_gray_vector[0], 0, NULL, NULL);

	lodepng::encode("imgs/test.png", im0_gray_vector, w, h, LCT_GREY);
	//WriteImage("imgs/Resize_test.png", im0_gray_vector, new_w, new_h, LCT_GREY, 0);
	



	printf("\nDone! Enter something to exit: ");
	getchar();
	return 0;
}