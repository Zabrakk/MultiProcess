#include <vector>
#include <math.h>
#include <algorithm>
#include "lodepng.h"
#include "ImageFunctions.h"
#include "OpenCLFunctions.h"

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

#define WINDOW_Y 13 
#define WINDOW_X 11 
#define MIN_DISPARITY 0
#define MAX_DISPARITY 65 // Scaled down. 260/4 as stated in the Assignment
#define THRESHOLD 3

int main() {
	// Set image dimensions
	unsigned w = 2940;
	unsigned h = 2016;
	unsigned new_w = floor(w / 4);
	unsigned new_h = floor(h / 4);
	// Initialize original image vector
	std::vector<unsigned char> im0, im1;

	// Read the images into memory
	if (ReadImage(im0, "im0.png", w, h)) return 1;
	if (ReadImage(im1, "im1.png", w, h)) return 1;
	printf("\n");


	// Initialize kernel sources
	kernel_source resize_grayscale_src, calc_zncc_src, cross_check_src, occlusion_fill_src, normalize_src;
	// Load Kernel sources into memory
	printf("Loadng Kernel sources from .cl files\n");
	if(!loadKernel(KERNEL_RESIZE_GRAYSCALE_FILE_NAME, &resize_grayscale_src));
	if(!loadKernel(KERNEL_CALCZNCC_FILE_NAME, &calc_zncc_src));
	if(!loadKernel(KERNEL_CROSS_CHECK_FILE_NAME, &cross_check_src));
	if(!loadKernel(KERNEL_OCCLUSION_FILL_FILE_NAME, &occlusion_fill_src));
	if(!loadKernel(KERNEL_NORMALIZE_FILE_NAME, &normalize_src));

	// Device selection + context and command queue creation
	int err_num;
	cl_device_id device_id = getGPUDevice();
	printf("Creating context\n");
	cl_context context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	printf("Creating command queue\n");
	cl_command_queue cmd_q = clCreateCommandQueue(context, device_id, CL_QUEUE_PROFILING_ENABLE, &err_num);
	if (!errorCheck(err_num)) return 1;

	// 2D image object creation for resize + grayscale
	printf("Creating 2D RGBA image objects for im0 and im1\n");
	cl_mem im0_cl = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &getRGBAImageFormat(), w, h, 0, &im0[0], &err_num);
	if (!errorCheck(err_num)) return 1;
	cl_mem im1_cl = clCreateImage2D(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &getRGBAImageFormat(), w, h, 0, &im1[0], &err_num);
	if (!errorCheck(err_num)) return 1;

	// 2D image objects for the result of resize + grayscale
	printf("Creating 2D grey image objects for the resized and grayscaled im0 and im1\n");
	cl_mem im0_gray_cl = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &getGrayImageFormat(), new_w, new_h, 0, NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	cl_mem im1_gray_cl = clCreateImage2D(context, CL_MEM_WRITE_ONLY, &getGrayImageFormat(), new_w, new_h, 0, NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	printf("\n");


	// Resize & Grayscale im0 and im1
	// Initialize parameters
	cl_kernel kernel;
	std::vector<unsigned char> im0_gray, im1_gray;
	// Create the resize & grayscale kernel
	kernel = createKernel(context, device_id, KERNEL_RESIZE_GRAYSCALE, (const char**)&resize_grayscale_src.source_str, (const size_t*)&resize_grayscale_src.source_size);
	
	// Give im0 parameters to the kernel
	printf("Using Resize & Grayscale kernel on im0\n");
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &im0_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &im0_gray_cl);
	if (!errorCheck(err_num)) return 1;
	// Execute the kernel
	im0_gray = executeImageKernel(cmd_q, kernel, new_w, new_h, im0_gray_cl);
	// Save result
	WriteImage(im0_gray, "imgs/im0_grey.png", new_w, new_h, LCT_GREY, 8);
	
	// Give im1 parameters to the kernel
	printf("Using Resize & Grayscale kernel on im1\n");
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &im1_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &im1_gray_cl);
	if (!errorCheck(err_num)) return 1;
	// Execute the kernel
	im1_gray = executeImageKernel(cmd_q, kernel, new_w, new_h, im1_gray_cl);
	// Save result
	WriteImage(im1_gray, "imgs/im1_grey.png", new_w, new_h, LCT_GREY, 8);
	printf("\n");

	// Free the unnecessary image objects from memory
	err_num = clReleaseMemObject(im0_cl);
	err_num |= clReleaseMemObject(im1_cl);
	if (!errorCheck(err_num)) return 1;
	FreeImageVector(im0);
	FreeImageVector(im1);


	// CalcZNCC
	// Initialize related parameters
	std::vector<unsigned char> dmap0(new_w * new_h);
	std::vector<unsigned char> dmap1(new_w * new_h);
	int min_disparity = 0;
	int max_disparity = 65;
	int neg_max_disparity = max_disparity * -1;
	int window_x = 11, window_y = 13;
	size_t global_size[] = { new_w, new_h, };
	size_t local_size[] = { 1, 1 };
	
	// Create Kernel
	kernel = createKernel(context, device_id, KERNEL_CALCZNCC, (const char**)&calc_zncc_src.source_str, (const size_t*)&calc_zncc_src.source_size);
	if (kernel == NULL) return 1;

	// Create memory objects
	im0_gray_cl = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, new_w * new_h * sizeof(unsigned char), &im0_gray[0], &err_num);
	if (!errorCheck(err_num)) return 1;
	im1_gray_cl = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, new_w * new_h * sizeof(unsigned char), &im1_gray[0], &err_num);
	if (!errorCheck(err_num)) return 1;
	cl_mem dmap0_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;
	cl_mem dmap1_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;

	/*
	* NOTES FROM OPENCL PDF
	* -Vectorizing code improves memory bandwidth and provides better coalescing of memory access (pp.99)
	* ->Done with vector data types (instead of scalar)
	* "Vectorization in OpenCL" - https://cs.anu.edu.au/courses/acceleratorsHPC/slides/OpenCLVectorization.pdf
	* "OpenCL When to use global, private, local, constant address spaces" - https://stackoverflow.com/questions/45426212/opencl-when-to-use-global-private-local-constant-address-spaces
	* ->Use local when all local workers access global memory
	* https://stackoverflow.com/questions/18217512/do-global-work-size-and-local-work-size-have-any-effect-on-application-logic
	*/
	
	// im0 left + im1 right parameters
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &im0_gray_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &im1_gray_cl);
	err_num |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &dmap0_cl);
	err_num |= clSetKernelArg(kernel, 3, sizeof(int), &min_disparity);
	err_num |= clSetKernelArg(kernel, 4, sizeof(int), &max_disparity);
	if (!errorCheck(err_num)) return 1;
	// Run the kernel
	dmap0 = executeBufferKernel(cmd_q, kernel, global_size, local_size, new_w, new_h, dmap0_cl);
	// Save the result
	WriteImage(dmap0, "imgs/im0_zncc.png", new_w, new_h, LCT_GREY, 8);
	
	// im1 left + im0 right parameters
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &im1_gray_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &im0_gray_cl);
	err_num |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &dmap1_cl);
	err_num |= clSetKernelArg(kernel, 3, sizeof(int), &neg_max_disparity);
	err_num |= clSetKernelArg(kernel, 4, sizeof(int), &min_disparity);
	if (!errorCheck(err_num)) return 1;
	// Run the kernel
	dmap1 = executeBufferKernel(cmd_q, kernel, global_size, local_size, new_w, new_h, dmap1_cl);
	// Save the result
	WriteImage(dmap1, "imgs/im1_zncc.png", new_w, new_h, LCT_GREY, 8);

	printf("\n");
	
	//
	// CrossCheck
	//
	std::vector<unsigned char> cross(new_w * new_h);
	unsigned int threshold = 3;
	// Create Kernel
	kernel = createKernel(context, device_id, KERNEL_CROSS_CHECK, (const char**)&cross_check_src.source_str, (const size_t*)&cross_check_src.source_size);
	if (kernel == NULL) return 1;

	// Create Buffers for the vector
	cl_mem cross_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);
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
	cross = executeBufferKernel(cmd_q, kernel, global_size, local_size, new_w, new_h, cross_cl);
	WriteImage(cross, "imgs/cross_check.png", new_w, new_h, LCT_GREY, 8);
	printf("\n");

	//
	// Occlusion Fill
	//
	std::vector<unsigned char> fill(new_w * new_h);
	// Create Kernel
	kernel = createKernel(context, device_id, KERNEL_OCCLUSION_FILL, (const char**)&occlusion_fill_src.source_str, (const size_t*)&occlusion_fill_src.source_size);
	if (kernel == NULL) return 1;

	// Create Buffers for the vectors
	cl_mem fill_cl = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);
	if (!errorCheck(err_num)) return 1;

	// Give parameters to Kernel
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &cross_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &fill_cl);
	err_num |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &new_h);
	if (!errorCheck(err_num)) return 1;
	// Execute the Kernel
	printf("Executing the Occlusion Fill Kernel\n");
	fill = executeBufferKernel(cmd_q, kernel, global_size, local_size, new_w, new_h, fill_cl);
	WriteImage(fill, "imgs/occlusion_fill.png", new_w, new_h, LCT_GREY, 8);
	printf("\n");

	//
	// Normalize the images
	//
	// Create Kernel
	kernel = createKernel(context, device_id, KERNEL_NORMALIZE, (const char**)&normalize_src.source_str, (const size_t*)&normalize_src.source_size);
	// Initialize
	int min, max;
	cl_mem dmap0_norm = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);;
	cl_mem dmap1_norm = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);;
	cl_mem cross_norm = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);;
	cl_mem fill_norm = clCreateBuffer(context, CL_MEM_READ_WRITE, new_w * new_h * sizeof(unsigned char), NULL, &err_num);;
	// dmap0
	min = *std::min_element(dmap0.begin(), dmap0.end());
	max = *std::max_element(dmap0.begin(), dmap0.end());
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dmap0_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &dmap0_norm);
	err_num |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 3, sizeof(int), &min);
	err_num |= clSetKernelArg(kernel, 4, sizeof(int), &max);
	printf("Normalizing dmap0");
	dmap0 = executeBufferKernel(cmd_q, kernel, global_size, local_size, new_w, new_h, dmap0_norm);
	WriteImage(dmap0, "imgs/im0_zncc_norm.png", new_w, new_h, LCT_GREY, 8);
	// dmap1
	min = *std::min_element(dmap1.begin(), dmap1.end());
	max = *std::max_element(dmap1.begin(), dmap1.end());
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dmap1_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &dmap1_norm);
	err_num |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 3, sizeof(int), &min);
	err_num |= clSetKernelArg(kernel, 4, sizeof(int), &max);
	printf("Normalizing dmap1");
	dmap1 = executeBufferKernel(cmd_q, kernel, global_size, local_size, new_w, new_h, dmap1_norm);
	WriteImage(dmap1, "imgs/im1_zncc_norm.png", new_w, new_h, LCT_GREY, 8);
	// Cross Check
	min = *std::min_element(cross.begin(), cross.end());
	max = *std::max_element(cross.begin(), cross.end());
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &cross_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &cross_norm);
	err_num |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 3, sizeof(int), &min);
	err_num |= clSetKernelArg(kernel, 4, sizeof(int), &max);
	printf("Normalizing Cross Check");
	cross = executeBufferKernel(cmd_q, kernel, global_size, local_size, new_w, new_h, cross_norm);
	WriteImage(cross, "imgs/cross_check_norm.png", new_w, new_h, LCT_GREY, 8);
	// Occlusion Fill
	min = *std::min_element(fill.begin(), fill.end());
	max = *std::max_element(fill.begin(), fill.end());
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &fill_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &fill_norm);
	err_num |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 3, sizeof(int), &min);
	err_num |= clSetKernelArg(kernel, 4, sizeof(int), &max);
	printf("Normalizing Occlusion Fill");
	fill = executeBufferKernel(cmd_q, kernel, global_size, local_size, new_w, new_h, fill_norm);
	WriteImage(fill, "imgs/occlusion_fill_norm.png", new_w, new_h, LCT_GREY, 8);

	//
	// Free Memory
	//
	free(resize_grayscale_src.source_str);
	free(calc_zncc_src.source_str);
	free(cross_check_src.source_str);
	free(occlusion_fill_src.source_str);
	free(normalize_src.source_str);

	err_num = clFlush(cmd_q);
	err_num |= clFinish(cmd_q);
	err_num |= clReleaseKernel(kernel);
	err_num |= clReleaseMemObject(im0_gray_cl);
	err_num |= clReleaseMemObject(im1_gray_cl);
	err_num |= clReleaseMemObject(dmap0_cl);
	err_num |= clReleaseMemObject(dmap1_cl);
	err_num |= clReleaseMemObject(cross_cl);
	err_num |= clReleaseMemObject(fill_cl);
	err_num |= clReleaseDevice(device_id);
	err_num |= clReleaseCommandQueue(cmd_q);
	err_num |= clReleaseContext(context);

	FreeImageVector(im0_gray);
	FreeImageVector(im1_gray);
	FreeImageVector(dmap0);
	FreeImageVector(dmap1);
	FreeImageVector(cross);
	FreeImageVector(fill);

	printf("DONE!\n");
	getchar();
	return 0;
}