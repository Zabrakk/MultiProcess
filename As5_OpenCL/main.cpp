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
#include <algorithm>


#define KERNEL_RESIZE_GRAYSCALE_FILE_NAME "kernels/resize_grayscale.cl" // Kernel file name
#define KERNEL_RESIZE_GRAYSCALE "resize_and_grayscale"

#define KERNEL_CALCZNCC_FILE_NAME "kernels/calc_zncc.cl"
#define KERNEL_CALCZNCC "calc_zncc"

#define KERNEL_CROSS_CHECK_FILE_NAME "kernels/cross_check.cl"
#define KERNEL_CROSS_CHECK "cross_check"

#define KERNEL_OCCLUSION_FILL_FILE_NAME "kernels/occlusion_fill.cl"
#define KERNEL_OCCLUSION_FILL "occlusion_fill"

#define KERNEL_NORMALIZE_FILE_NAME "kernels/normalize.cl"
#define KERNEL_NORMALIZE "normalize_img"

int main() {
	// Image width and height. Both input images are the same size
	unsigned w = 2940;
	unsigned h = 2016;
	int window_y = 13;
	int window_x = 11;
	int min_disparity = 0;
	int max_disparity = 65;
	int neg_max_disparity = max_disparity * -1;
	int threshold = 3;
	// Create output format and object for the resized grayscaled image
	int new_w = floor(w / 4);
	int new_h = floor(h / 4);
	// Grayscaled vectors
	std::vector<unsigned char> im0_gray_vector(new_w * new_h);
	std::vector<unsigned char> im1_gray_vector(new_w * new_h);
	std::vector<unsigned char> dmap0(new_w * new_h);
	std::vector<unsigned char> dmap1(new_w * new_h);
	std::vector<unsigned char> cross(new_w * new_h);
	std::vector<unsigned char> fill(new_w * new_h);

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
	delete im0;
	delete im1;
	
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

	cl_mem im0_cl = NULL, im1_cl = NULL, im0_gray_cl = NULL, im1_gray_cl = NULL, dmap0_cl = NULL, dmap1_cl = NULL, cross_cl = NULL, fill_cl = NULL, normalized = NULL;

	// Timing varaibles
	double milliseconds = 0;
	cl_ulong opencl_start = 0, opencl_end = 0;

	// For image object reading
	size_t origin[3] = { 0, 0, 0 };
	size_t region[3] = { new_w, new_h, 1 };
	// Kernel workgroup sizes
	size_t global_work_size[] = { new_w, new_h }; // Use the downscaled image height and width
	size_t local_work_size[] = { 1, 1 };

	// Load Kernel sources into memory
	kernel_source resize_grayscale_src = loadKernel(KERNEL_RESIZE_GRAYSCALE_FILE_NAME);
	if (resize_grayscale_src.ok == 0) return 1;
	kernel_source calc_zncc_src = loadKernel(KERNEL_CALCZNCC_FILE_NAME);
	if (calc_zncc_src.ok == 0) return 1;
	kernel_source cross_check_src = loadKernel(KERNEL_CROSS_CHECK_FILE_NAME);
	if (cross_check_src.ok == 0) return 1;
	kernel_source occlusion_fill_src = loadKernel(KERNEL_OCCLUSION_FILL_FILE_NAME);
	if (occlusion_fill_src.ok == 0) return 1;
	kernel_source normalize_src = loadKernel(KERNEL_NORMALIZE_FILE_NAME);
	if (normalize_src.ok == 0) return 1;
	printf("\n");

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
	kernel = createKernel(context, device_id, KERNEL_RESIZE_GRAYSCALE, (const char**)&resize_grayscale_src.source_str, (const size_t*)&resize_grayscale_src.source_size);

	// Format for the OpenCL image objects
	cl_image_format format;
	format.image_channel_order = CL_RGBA; // The image was read as RGBA
	format.image_channel_data_type = CL_UNORM_INT8;

	// Create a 2D image object for im0 and im1
	printf("Creating 2D images objects for im0 and im1\n");
	im0_cl = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &format, w, h, 0, &im0_vector[0], &err_num);
	if (!errorCheck(err_num)) return 1;
	im1_cl = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &format, w, h, 0, &im1_vector[0], &err_num);
	if (!errorCheck(err_num)) return 1;

	// Create format for grayscale image objects
	cl_image_format format_gray;
	format_gray.image_channel_order = CL_LUMINANCE; // The image was is in grayscale
	format_gray.image_channel_data_type = CL_UNORM_INT8;

	im0_gray_cl = clCreateImage2D(context, CL_MEM_READ_WRITE, &format_gray, new_w, new_h, 0, NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	im1_gray_cl = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &format_gray, new_w, new_h, 0, NULL, &err_num);
	if (!errorCheck(err_num)) return 1;

	printf("\n");

	/****
	* Resize + Grayscale im0
	****/

	// Give parameters to Kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &im0_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &im0_gray_cl);
	if (!errorCheck(err_num)) return 1;

	// Execute the Kernel
	printf("Executing the resize and grayscale Kernel for im0\n");
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);
	
	// Get the resulting grayscaled image
	err_num = clEnqueueReadImage(cmd_q, im0_gray_cl, CL_TRUE, origin, region, 0, 0, &im0_gray_vector[0], 0, NULL, NULL);

	if (WriteImage("imgs/im0_gray.png", im0_gray_vector, new_w, new_h, LCT_GREY, 8)) return 1;
	
	/****
	* Resize + Grayscale im1
	****/

	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &im1_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &im1_gray_cl);
	if (!errorCheck(err_num)) return 1;

	// Execute the Kernel
	printf("Executing the resize and grayscale Kernel for im1\n");
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);

	// Get the resulting grayscaled image
	err_num = clEnqueueReadImage(cmd_q, im1_gray_cl, CL_TRUE, origin, region, 0, 0, &im1_gray_vector[0], 0, NULL, NULL);

	if (WriteImage("imgs/im1_gray.png", im1_gray_vector, new_w, new_h, LCT_GREY, 8)) return 1;

	/*
	* Free memory
	*/

	FreeImageVector(im0_vector);
	FreeImageVector(im1_vector);


	/**********************************************/
	/*				  CalcZNCC                    */
	/**********************************************/

	// Create Kernel
	kernel = createKernel(context, device_id, KERNEL_CALCZNCC, (const char**)&calc_zncc_src.source_str, (const size_t*)&calc_zncc_src.source_size);
	if (kernel == NULL) return 1;

	// Create Buffers for the vectors
	im0_gray_cl = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, new_w * new_h * sizeof(unsigned char), &im0_gray_vector[0], &err_num);
	if (!errorCheck(err_num)) return 1;
	im1_gray_cl = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, new_w * new_h * sizeof(unsigned char), &im1_gray_vector[0], &err_num);
	if (!errorCheck(err_num)) return 1;
	dmap0_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	dmap1_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;

	/****
	* im0 left + im1 right
	****/
	// Give parameters to Kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &im0_gray_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &im1_gray_cl);
	err_num |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &dmap0_cl);
	err_num |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &new_h);
	err_num |= clSetKernelArg(kernel, 5, sizeof(int), &window_y);
	err_num |= clSetKernelArg(kernel, 6, sizeof(int), &window_x);
	err_num |= clSetKernelArg(kernel, 7, sizeof(int), &min_disparity);
	err_num |= clSetKernelArg(kernel, 8, sizeof(int), &max_disparity);
	if (!errorCheck(err_num)) return 1;

	// Execute the Kernel
	printf("Executing the CalcZNCC Kernel for im0=left & im1=right\n");
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);

	err_num = clEnqueueReadBuffer(cmd_q, dmap0_cl, CL_TRUE, 0, new_w * new_h * sizeof(unsigned char), &dmap0[0], 0, NULL, NULL);
	WriteImage("imgs/im0_disparity.png", dmap0, new_w, new_h, LCT_GREY, 8);

	/****
	* im1 left + im0 right
	****/
	// Give parameters to Kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &im1_gray_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &im0_gray_cl);
	err_num |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &dmap1_cl);
	err_num |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &new_h);
	err_num |= clSetKernelArg(kernel, 5, sizeof(int), &window_y);
	err_num |= clSetKernelArg(kernel, 6, sizeof(int), &window_x);
	err_num |= clSetKernelArg(kernel, 7, sizeof(int), &neg_max_disparity);
	err_num |= clSetKernelArg(kernel, 8, sizeof(int), &min_disparity);
	if (!errorCheck(err_num)) return 1;

	// Execute the Kernel
	printf("Executing the CalcZNCC Kernel for im0=left & im1=right\n");
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);

	err_num = clEnqueueReadBuffer(cmd_q, dmap1_cl, CL_TRUE, 0, new_w * new_h * sizeof(unsigned char), &dmap1[0], 0, NULL, NULL);
	WriteImage("imgs/im1_disparity.png", dmap1, new_w, new_h, LCT_GREY, 8);

	/**********************************************/
	/*				  CrossCheck                  */
	/**********************************************/

	// Create Kernel
	kernel = createKernel(context, device_id, KERNEL_CROSS_CHECK, (const char**)&cross_check_src.source_str, (const size_t*)&cross_check_src.source_size);
	if (kernel == NULL) return 1;

	// Create Buffers for the vectors
	cross_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;

	// Give parameters to Kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dmap0_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &dmap1_cl);
	err_num |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &cross_cl);
	err_num |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &new_h);
	err_num |= clSetKernelArg(kernel, 5, sizeof(unsigned int), &threshold);
	if (!errorCheck(err_num)) return 1;

	// Execute the Kernel
	printf("Executing the CrossCheck Kernel\n");
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);

	err_num = clEnqueueReadBuffer(cmd_q, cross_cl, CL_TRUE, 0, new_w * new_h * sizeof(unsigned char), &cross[0], 0, NULL, NULL);
	WriteImage("imgs/cross_check.png", cross, new_w, new_h, LCT_GREY, 8);

	/**********************************************/
	/*				Occlusion Fill                */
	/**********************************************/

	// Create Kernel
	kernel = createKernel(context, device_id, KERNEL_OCCLUSION_FILL, (const char**)&occlusion_fill_src.source_str, (const size_t*)&occlusion_fill_src.source_size);
	if (kernel == NULL) return 1;
	
	// Create Buffers for the vectors
	fill_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;

	// Give parameters to Kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &cross_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &fill_cl);
	err_num |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &new_h);
	if (!errorCheck(err_num)) return 1;

	// Execute the Kernel
	printf("Executing the Occlusion Fill Kernel\n");
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);

	err_num = clEnqueueReadBuffer(cmd_q, fill_cl, CL_TRUE, 0, new_w * new_h * sizeof(unsigned char), &fill[0], 0, NULL, NULL);
	WriteImage("imgs/occlusion_fill.png", fill, new_w, new_h, LCT_GREY, 8);

	/**********************************************/
	/*				  Normalize                   */
	/**********************************************/

	unsigned int min = 0, max = 0;
	normalized = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;

	// Create Kernel
	kernel = createKernel(context, device_id, KERNEL_NORMALIZE, (const char**)&normalize_src.source_str, (const size_t*)&normalize_src.source_size);
	if (kernel == NULL) return 1;

	// dmap0

	min = *std::min_element(dmap0.begin(), dmap0.end());
	max = *std::max_element(dmap0.begin(), dmap0.end());
	// Give parameters to Kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dmap0_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &normalized);
	err_num |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &min);
	err_num |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &max);
	if (!errorCheck(err_num)) return 1;
	
	// Execute the Kernel
	printf("Executing the Normalization Kernel on dmap0\n");;
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);

	err_num = clEnqueueReadBuffer(cmd_q, normalized, CL_TRUE, 0, new_w * new_h * sizeof(unsigned char), &dmap0[0], 0, NULL, NULL);
	WriteImage("imgs/im0_disparity_norm.png", dmap0, new_w, new_h, LCT_GREY, 8);

	// dmap1

	min = *std::min_element(dmap1.begin(), dmap1.end());
	max = *std::max_element(dmap1.begin(), dmap1.end());
	// Give parameters to Kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dmap1_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &normalized);
	err_num |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &min);
	err_num |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &max);
	if (!errorCheck(err_num)) return 1;

	// Execute the Kernel
	printf("Executing the Normalization Kernel on dmap1\n");
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);

	err_num = clEnqueueReadBuffer(cmd_q, normalized, CL_TRUE, 0, new_w * new_h * sizeof(unsigned char), &dmap1[0], 0, NULL, NULL);
	WriteImage("imgs/im1_disparity_norm.png", dmap1, new_w, new_h, LCT_GREY, 8);

	// Cross Check

	min = *std::min_element(cross.begin(), cross.end());
	max = *std::max_element(cross.begin(), cross.end());
	// Give parameters to Kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &cross_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &normalized);
	err_num |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &min);
	err_num |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &max);
	if (!errorCheck(err_num)) return 1;

	// Execute the Kernel
	printf("Executing the Normalization Kernel on Cross Check image\n");
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);

	err_num = clEnqueueReadBuffer(cmd_q, normalized, CL_TRUE, 0, new_w * new_h * sizeof(unsigned char), &cross[0], 0, NULL, NULL);
	WriteImage("imgs/cross_check_norm.png", cross, new_w, new_h, LCT_GREY, 8);

	// Occlusion fill

	min = *std::min_element(fill.begin(), fill.end());
	max = *std::max_element(fill.begin(), fill.end());
	// Give parameters to Kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &fill_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &normalized);
	err_num |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &min);
	err_num |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &max);
	if (!errorCheck(err_num)) return 1;

	// Execute the Kernel
	printf("Executing the Normalization Kernel on Occlusion Fill image\n");
	err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return 1;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);

	err_num = clEnqueueReadBuffer(cmd_q, normalized, CL_TRUE, 0, new_w * new_h * sizeof(unsigned char), &fill[0], 0, NULL, NULL);
	WriteImage("imgs/occlusion_fill_norm.png", fill, new_w, new_h, LCT_GREY, 8);

	/**********************************************/
	/*				  Free Memory                 */
	/**********************************************/

	FreeImageVector(im0_gray_vector);
	FreeImageVector(im1_gray_vector);
	FreeImageVector(dmap0);
	FreeImageVector(dmap1);
	FreeImageVector(cross);
	FreeImageVector(fill);

	err_num = clFlush(cmd_q);
	err_num |= clFinish(cmd_q);
	err_num |= clReleaseEvent(event);
	err_num |= clReleaseKernel(kernel);
	err_num |= clReleaseMemObject(im0_cl);
	err_num |= clReleaseMemObject(im1_cl);
	err_num |= clReleaseMemObject(im0_gray_cl);
	err_num |= clReleaseMemObject(im1_gray_cl);
	err_num |= clReleaseMemObject(dmap0_cl);
	err_num |= clReleaseMemObject(dmap1_cl);
	err_num |= clReleaseMemObject(cross_cl);
	err_num |= clReleaseMemObject(fill_cl);
	err_num |= clReleaseMemObject(normalized);
	err_num |= clReleaseDevice(device_id);
	err_num |= clReleaseCommandQueue(cmd_q);
	err_num |= clReleaseContext(context);

	free(resize_grayscale_src.source_str);
	free(calc_zncc_src.source_str);
	free(cross_check_src.source_str);
	free(occlusion_fill_src.source_str);
	free(normalize_src.source_str);

	printf("\nDone! Enter something to exit: ");
	getchar();
	return 0;
}