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
#define GRAYSCALE_NAME "to_grayscale" // Kernel Function name
#define MASK_NAME "apply_mask"


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

	unsigned int w = 2988; // Image width
	unsigned int h = 2008; // Image height
	char* file_name = "im0.png"; // Name of the image file
	LARGE_INTEGER start, end, elapsed;
	LARGE_INTEGER freq;
	std::vector<unsigned char> img; // This vector will have w * h * 3 elements representing R,G,B
	std::vector<unsigned char> grayscaled(w * h); // w * h sice doesn't include the three colors, only their sum
	std::vector<unsigned char> result(w * h); // The result after the mask goes here

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
	cl_mem grayscaled_mem = NULL;
	cl_mem result_mem = NULL;

	// Load Kernel source into memory
	kernel_source src = loadKernel(KERNEL_FINE_NAME);
	if (src.ok == 0) return 1;

	printf("Selecting platform and device\n");
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
	printf("\n");

	/**********************************************/
	/*				  GRAYSCALE                   */
	/**********************************************/

	// Create Kernel
	kernel = createKernel(context, device_id, GRAYSCALE_NAME, (const char**)&src.source_str, (const size_t*)&src.source_size);
	if (kernel == NULL) return 1;
	// Create memory buffer for original and grayscaled image
	printf("Creating memory objects\n");
	original_mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, w * h * 3 * sizeof(unsigned char), &img[0], &err_num);
	if (!errorCheck(err_num)) return 1;
	grayscaled_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, w * h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	// Give the original and grayscaled image as parameters to the kernel
	printf("Passing arguments to Kernel\n");
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &original_mem);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &grayscaled_mem);
	if (!errorCheck(err_num)) return 1;
	// Set global and local work sizes
	size_t global_work_size[1] = { w*h };
	size_t local_work_size[1] = { 18 }; 
	// Execute the Kernel, give event so execution time can be obtained
	printf("Executing grayscale Kernel\n");
	// Only using 1 dimention for this
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, &event);
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
	err_num = clEnqueueReadBuffer(cmd_q, grayscaled_mem, CL_TRUE, 0, w * h * sizeof(unsigned char), &grayscaled[0], 0, NULL, NULL);
	if (!errorCheck(err_num)) return 1;

	// Free the memory used for the original image
	printf("Freeing memory used for the original image\n");
	err_num = clReleaseMemObject(original_mem);
	if (!errorCheck(err_num)) return 1;
	img = std::vector<unsigned char>();
	printf("\n");

	/**********************************************/
	/*				  5x5 MASK                    */
	/**********************************************/

	// Create Kernel
	kernel = createKernel(context, device_id, MASK_NAME, (const char**)&src.source_str, (const size_t*)&src.source_size);
	if (kernel == NULL) return 1;
	printf("Creating memory objects\n");
	grayscaled_mem = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, w * h * sizeof(unsigned char), &grayscaled[0], &err_num);
	if (!errorCheck(err_num)) return 1;
	result_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, w * h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	printf("Passing arguments to Kernel\n");
	// Give the grayscaled image as parameters to the kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &grayscaled_mem);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &result_mem);
	err_num |= clSetKernelArg(kernel, 2, sizeof(cl_uint), &w); // No need for memory object
	err_num |= clSetKernelArg(kernel, 3, sizeof(cl_uint), &h);
	if (!errorCheck(err_num)) return 1;

	// Set global and local work sizes
	size_t global_work_size2[] = { w, h }; // x, y
	size_t local_work_size2[] = { 4, 4 };

	// Notice that workdim = 2!!!
	printf("Executing the 5x5 mask Kernel\n");
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size2, local_work_size2, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);
	// Read the result
	err_num = clEnqueueReadBuffer(cmd_q, result_mem, CL_TRUE, 0, w * h * sizeof(unsigned char), &result[0], 0, NULL, NULL);
	if (!errorCheck(err_num)) return 1;

	// Free the memory used for the grayscaled image
	printf("Freeing memory used for the grayscaled image\n");
	err_num = clReleaseMemObject(grayscaled_mem);
	if (!errorCheck(err_num)) return 1;
	grayscaled = std::vector<unsigned char>();
	printf("\n");

	/**********************************************/
	/*				  SAVE IMAGE                  */
	/**********************************************/

	printf("Saving the resulting image as result.png\n");
	// Start timer
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&start);
	// Encode as PNG, and save to "result.png"
	lodepng::encode("result.png", result, w, h, LCT_GREY); // grayscaled
	// Stop timer
	QueryPerformanceCounter(&end);
	elapsed.QuadPart = end.QuadPart - start.QuadPart;
	elapsed.QuadPart *= 1000000;
	elapsed.QuadPart /= freq.QuadPart;
	printf("Image saved. Took %ld microseconds\n\n", elapsed);
	

	/**********************************************/
	/*				  FINALIZATION                */
	/**********************************************/

	printf("Finalizing by freeing rest of the memory objects\n");
	err_num = clFlush(cmd_q);
	err_num |= clFinish(cmd_q);
	err_num |= clReleaseEvent(event);
	err_num |= clReleaseKernel(kernel);
	err_num |= clReleaseMemObject(result_mem);
	err_num |= clReleaseDevice(device_id);
	err_num |= clReleaseCommandQueue(cmd_q);
	err_num |= clReleaseContext(context);

	free(src.source_str);
	result = std::vector<unsigned char>();

	printf("Done. Enter a char to exit!\n");
	getchar();
	return 0;
}