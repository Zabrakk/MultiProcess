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
#include "OpenCLFunctions.h"

#define KERNEL_FINE_NAME "kernel.cl" // Kernel file name
#define KERNEL_NAME "to_grayscale" // Kernel Function name

/* The code must do the following
* 1. Load image "im0.png" with lodepng.cpp (DONE)
* 2. Convert  to gray scale with a custom function
* 3. Apply 5x5 moving filter on the gray scaled image with a custom function
* 4. Save final image, use lodepng.cpp
* 6. Display platform/device information
* 7. Display intermediate results for steps 1-4
* 8. Use profiling to get execution times for each of the tasks 1-4, display result
* 9. Measure memory usage (OPTIONAL)
*/

/* Sources:
* HelloWorld Report.pdf from the course
* OpenCL Programming Guide provided on the course
* "Acquiring high-resolution time stamps" - https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
* LodePNG Decode Example - https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_decode.cpp
* LodePNG Encode Example - https://raw.githubusercontent.com/lvandeve/lodepng/master/examples/example_encode.cpp
*/

int main() {

	/**********************************************/
	/*				  LOAD IMAGE                  */
	/**********************************************/

	LARGE_INTEGER start, end, elapsed;
	LARGE_INTEGER freq;
	unsigned w = 2988; // Image width
	unsigned h = 2008; // Image height
	char* file_name = "im0.png"; // Name of the image file
	std::vector<unsigned char> img; // This vector will have w * h * 4 elements representing R,G,B,A
	std::vector<unsigned char> red(w * h);
	std::vector<unsigned char> green(w * h);
	std::vector<unsigned char> blue(w * h);
	std::vector<unsigned char> alpha(w * h);
	std::vector<unsigned char> grayscaled(w * h); // Includes the alpha channel so ==> width * height * 2 // ADD * 2 HERE
	std::vector<unsigned char> result; // The result after the mask goes here

	printf("Loading %s to memory\n", file_name);
	// Start measurint elapsed time with WINAPI
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);

	// Loads and decodes. 4 bytes per pixel, ordered R,G,B,R,G,B, ...
	if (lodepng::decode(img, w, h, (const char*)file_name, LCT_RGB) != 0) {
		printf("Failed to load %s\n\n", file_name);
		getchar();
		return 1;
	}

	// Stop measuring, image loaded
	QueryPerformanceCounter(&end);
	elapsed.QuadPart = end.QuadPart - start.QuadPart;
	elapsed.QuadPart *= 1000000;
	elapsed.QuadPart /= freq.QuadPart;
	printf("%s loaded to memory. Took %ld microseconds\n\n", file_name, elapsed);

	// Divide image into multiple arrays
	int j = 0;
	for (int i = 0; i < img.size(); i += 4) {
		red[j] = img[i];
		green[j] = img[i + 1];
		blue[j] = img[i + 2];
		alpha[j] = img[i + 3];
	}

	/* GRAYSCALE THE IMAGE
	int c = -1, j = 0;
	for (int i = 0; i < img.size(); i += 4) {
		grayscaled[j] = (unsigned char)((float)img[i] * 0.299 + (float)img[i + 1] * 0.587 + (float)img[i + 2] * 0.114);
		grayscaled[j + 1] = img[i + 3];
		j += 2;
	}
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

	cl_mem original_mem = NULL;
	/*
	cl_mem red_mem = NULL;
	cl_mem green_mem = NULL;
	cl_mem blue_mem = NULL;
	cl_mem alpha_mem = NULL;
	*/
	cl_mem grayscaled_mem = NULL;
	cl_mem result_mem = NULL;

	// Load Kernel source into memory
	kernel_source src = loadKernel(KERNEL_FINE_NAME);
	if (src.ok == 0) return 1;

	// Obtain first OpenCL platform
	err_num = clGetPlatformIDs(1, &platform_id, &num_of_platforms);
	if (!errorCheck(err_num)) return 1;
	// Print number of platforms
	printf("Number of platforms: %d\n", num_of_platforms);
	// Obtain first device from the selected platform with type GPU
	err_num = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &num_of_devices);
	// Print number of devices on the selected platform
	printf("Number of devices on the selected platform: %d\n\n", num_of_devices);
	if (!errorCheck(err_num)) return 1;
	// Create the context
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	// Print info about the selected device
	printDeviceInfo(device_id);
	// Create command queue with profiling enabled, so Kernel exewcution time can be obtained
	cmd_q = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &err_num);

	// Create memory buffer for original and grayscaled image
	original_mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, w * h * 3 * sizeof(unsigned char), &img[0], &err_num); // ADD *4 HERE?
	if (!errorCheck(err_num)) return 1;
	grayscaled_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, w * h * sizeof(unsigned char), NULL, &err_num); // ADD * 2 HERE?
	if (!errorCheck(err_num)) return 1;

	/*
	red_mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, w * h * sizeof(unsigned char), &red, &err_num);
	if (!errorCheck(err_num)) return 1;
	green_mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, w * h * sizeof(unsigned char), &green, &err_num);
	if (!errorCheck(err_num)) return 1;
	blue_mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, w * h * sizeof(unsigned char), &blue, &err_num);
	if (!errorCheck(err_num)) return 1;
	alpha_mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, w * h * sizeof(unsigned char), &alpha, &err_num);
	if (!errorCheck(err_num)) return 1;
	*/

	// Create the program with Kernel source
	program = clCreateProgramWithSource(context, 1, (const char**)&src.source_str, (const size_t*)&src.source_size, &err_num);
	if (!errorCheck(err_num)) return 1;
	// Build the program
	err_num = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	//Print logs if build failed
	if (err_num == CL_BUILD_PROGRAM_FAILURE) {
		size_t log_size;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		// Allocate memory for the log
		char* log = (char*)malloc(log_size);
		// Get the log
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		// Print the log
		printf("%s\n", log);
	}
	// Create the actual Kernel
	kernel = clCreateKernel(program, KERNEL_NAME, &err_num);
	if (!errorCheck(err_num)) return 1;
	// Give the original and grayscaled image as parameters to the kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &original_mem);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &grayscaled_mem);
	if (!errorCheck(err_num)) return 1;

	// Set global and loval work sizes
	size_t global_work_size[1] = { w*h };
	size_t local_work_size[1] = { 3 }; // Each row is it's own local work group
	// Execute the Kernel, give event so execution time can be obtained
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	cl_ulong opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
	double milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	// Get the resulting grayscaled image
	err_num = clEnqueueReadBuffer(cmd_q, grayscaled_mem, CL_TRUE, 0, w * h * sizeof(unsigned char), &grayscaled[0], 0, NULL, NULL); // ADD * 2 HERE?
	if (!errorCheck(err_num)) return 1;
	// Output matrix and execution time
	printf("Kernel execution took %f milliseconds\n", milliseconds);

	/**********************************************/
	/*				  SAVE IMAGE                  */
	/**********************************************/

	
	printf("Saving the resulting image as result.png\n");
	// Start timer
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
	// Encode as PNG, and save to "result.png"
	lodepng::encode("result.png", grayscaled, w, h, LCT_GREY);
	// Stop timer
	QueryPerformanceCounter(&end);
	elapsed.QuadPart = end.QuadPart - start.QuadPart;
	elapsed.QuadPart *= 1000000;
	elapsed.QuadPart /= freq.QuadPart;
	printf("Image saved. Took %ld microseconds\n\n", elapsed);
	

	/**********************************************/
	/*				  FINALIZATION                */
	/**********************************************/

	//free(img);
	printf("Done. Enter a char to exit!\n");
	getchar();
	return 0;
}