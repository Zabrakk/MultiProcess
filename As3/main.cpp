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
#define WIDTH 2988
#define HEIGHT 2008

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
	std::vector<unsigned char> grayscaled(w * h); // Includes the alpha channel so ==> width * height * 2 // ADD * 2 HERE
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
	cl_mem width_mem = NULL;
	cl_mem height_mem = NULL;
	cl_mem result_mem = NULL;

	// Load Kernel source into memory
	kernel_source src = loadKernel(KERNEL_FINE_NAME);
	if (src.ok == 0) return 1;

	printf("Selecting platform/device\n");
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
	// Create Kernel
	kernel = createKernel(context, device_id, GRAYSCALE_NAME, (const char**)&src.source_str, (const size_t*)&src.source_size);
	if (kernel == NULL) return 1;

	/**********************************************/
	/*				  GRAYSCALE                   */
	/**********************************************/

	// Create memory buffer for original and grayscaled image
	original_mem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, w * h * 3 * sizeof(unsigned char), &img[0], &err_num);
	if (!errorCheck(err_num)) return 1;
	grayscaled_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, w * h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	// Give the original and grayscaled image as parameters to the kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &original_mem);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &grayscaled_mem);
	if (!errorCheck(err_num)) return 1;
	// Set global and local work sizes
	size_t global_work_size[1] = { w*h };
	size_t local_work_size[1] = { 18 }; 
	// Execute the Kernel, give event so execution time can be obtained
	printf("Executing grayscale Kernel function\n");
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	cl_ulong opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &end, NULL);
	double milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n\n", milliseconds);
	// Get the resulting grayscaled image
	err_num = clEnqueueReadBuffer(cmd_q, grayscaled_mem, CL_TRUE, 0, w * h * sizeof(unsigned char), &grayscaled[0], 0, NULL, NULL);
	if (!errorCheck(err_num)) return 1;


	/**********************************************/
	/*				  5x5 MASK                    */
	/**********************************************/
	// https://stackoverflow.com/questions/44915272/how-opencl-work-with-opencv
	// https://stackoverflow.com/questions/4880819/logic-for-padding-of-an-image-array

	const unsigned int width = 2998;
	const unsigned int height = 2008;

	// Create Kernel
	kernel = createKernel(context, device_id, MASK_NAME, (const char**)&src.source_str, (const size_t*)&src.source_size);
	if (kernel == NULL) return 1;
	grayscaled_mem = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, w * h * sizeof(unsigned char), &grayscaled[0], &err_num);
	if (!errorCheck(err_num)) return 1;
	result_mem = clCreateBuffer(context, CL_MEM_READ_WRITE, w * h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;

	// Give the grayscaled image as parameters to the kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &grayscaled_mem);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &result_mem);
	err_num |= clSetKernelArg(kernel, 2, sizeof(cl_uint), &width);
	err_num |= clSetKernelArg(kernel, 3, sizeof(cl_uint), &height);
	if (!errorCheck(err_num)) return 1;

	// Set global and local work sizes
	size_t global_work_size2[] = { w, h }; // x, y
	size_t local_work_size2[] = { 4, 4 };

	// Notice that workdim = 2!!!
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size2, local_work_size2, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	err_num = clEnqueueReadBuffer(cmd_q, result_mem, CL_TRUE, 0, w * h * sizeof(unsigned char), &result[0], 0, NULL, NULL);
	if (!errorCheck(err_num)) return 1;

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

	//free(img);
	printf("Done. Enter a char to exit!\n");
	getchar();
	return 0;
}