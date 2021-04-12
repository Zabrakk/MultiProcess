#include <vector>
#include <math.h>
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
	/*
	if(!loadKernel(KERNEL_CROSS_CHECK_FILE_NAME, &cross_check_src));
	if(!loadKernel(KERNEL_OCCLUSION_FILL_FILE_NAME, &occlusion_fill_src));
	if(!loadKernel(KERNEL_NORMALIZE_FILE_NAME, &normalize_src));
	*/

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
	cl_mem im0_gray_cl = clCreateImage2D(context, CL_MEM_READ_WRITE, &getGrayImageFormat(), new_w, new_h, 0, NULL, &err_num);
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
	im0_gray = executeGrayscaleKernel(cmd_q, kernel, new_w, new_h, im0_gray_cl);
	// Save result
	WriteImage(im0_gray, "imgs/im0_grey.png", new_w, new_h, LCT_GREY, 8);
	
	// Give im1 parameters to the kernel
	printf("Using Resize & Grayscale kernel on im1\n");
	err_num = clSetKernelArg(kernel, 0, sizeof(cl_mem), &im1_cl);
	err_num |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &im1_gray_cl);
	if (!errorCheck(err_num)) return 1;
	// Execute the kernel
	im1_gray = executeGrayscaleKernel(cmd_q, kernel, new_w, new_h, im1_gray_cl);
	// Save result
	WriteImage(im1_gray, "imgs/im1_grey.png", new_w, new_h, LCT_GREY, 8);
	printf("\n");

	// Free the unnecessary image objects from memory
	err_num = clReleaseMemObject(im0_cl);
	err_num |= clReleaseMemObject(im1_cl);
	err_num |= clReleaseMemObject(im0_gray_cl);
	err_num |= clReleaseMemObject(im1_gray_cl);
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
	
	// Create Kernel
	kernel = createKernel(context, device_id, KERNEL_CALCZNCC, (const char**)&calc_zncc_src.source_str, (const size_t*)&calc_zncc_src.source_size);
	if (kernel == NULL) return 1;
	// Create Buffers for the vectors
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
	err_num |= clSetKernelArg(kernel, 3, sizeof(unsigned int), &new_w);
	err_num |= clSetKernelArg(kernel, 4, sizeof(unsigned int), &new_h);
	err_num |= clSetKernelArg(kernel, 5, sizeof(int), &min_disparity);
	err_num |= clSetKernelArg(kernel, 6, sizeof(int), &max_disparity);
	if (!errorCheck(err_num)) return 1;
	// Run the kernel
	size_t global_size[] = { new_w, new_h, };
	size_t local_size[] = { 1, 1 }; 

	dmap0 = executeBufferKernel(cmd_q, kernel, global_size, local_size, new_w, new_h, dmap0_cl);
	// Save the result
	WriteImage(dmap0, "imgs/im0_zncc.png", new_w, new_h, LCT_GREY, 8);

	printf("\n");
	

	// Free kernel source char pointers
	free(resize_grayscale_src.source_str);
	free(calc_zncc_src.source_str);
	/*
	free(cross_check_src.source_str);
	free(occlusion_fill_src.source_str);
	free(normalize_src.source_str);
	*/

	printf("DONE!\n");
	getchar();
	return 0;
}