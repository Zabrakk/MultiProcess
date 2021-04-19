#include "OpenCLFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <vector>

/** Most of this file uses these sources:
*** HelloWord Report.pdf from the course
*** OpenCL Programming Guide from the course
**/

// OpenCL include
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif // __APPLE__

#define MAX_SOURCE_SIZE (0x001000)


int errorCheck(cl_int err_num) {
	switch (err_num) {
	case CL_SUCCESS:
		return 1;
	case CL_DEVICE_NOT_FOUND:
		printf("CL_DEVICE_NOT_FOUND\n");
		break;
	case CL_INVALID_CONTEXT:
		printf("CL_INVALID_CONTEXT\n");
		break;
	case CL_INVALID_KERNEL_NAME:
		printf("CL_INVALID_KERNEL_NAME\n");
		break;
	case CL_INVALID_VALUE:
		printf("CL_INVALID_VALUE\n");
		break;
	case CL_INVALID_ARG_SIZE:
		printf("CL_INVALID_ARG_SIZE\n");
		break;
	case CL_INVALID_MEM_OBJECT:
		printf("CL_INVALID_MEM_OBJECT\n");
		break;
	case CL_INVALID_WORK_GROUP_SIZE:
		printf("CL_INVALID_WORK_GROUP_SIZE\n");
		break;
	case CL_INVALID_HOST_PTR:
		printf("CL_INVALID_HOST_PTR\n");
		break;
	case CL_IMAGE_FORMAT_NOT_SUPPORTED:
		printf("CL_IMAGE_FORMAT_NOT_SUPPORTED\n");
		break;
	case CL_INVALID_IMAGE_DESCRIPTOR:
		printf("CL_INVALID_IMAGE_DESCRIPTOR\n");
		break;
	case CL_INVALID_IMAGE_SIZE:
		printf("CL_INVALID_IMAGE_SIZE\n");
		break;
	case CL_INVALID_KERNEL:
		printf("CL_INVALID_KERNEL\n");
		break;
	default:
		printf("An unexpected OpenCL error occured! Code was %d\n", err_num);
	}
	printf("Enter something to exit: ");
	getchar();
	return 0;
}

int loadKernel(char file_name[], kernel_source *src) {
	FILE* fp;
	//printf("Loading Kernel file %s\n", file_name);

	// Load source code, and store to kernel_source struct
	fopen_s(&fp, file_name, "r");
	if (!fp) {
		printf("Failed to load kernel, .cl file not found!\n");
		return 0;
	}

	src->source_str = (char*)calloc(MAX_SOURCE_SIZE, 1);
	if (src->source_str == 0) {
		printf("Kernel is empty (source_str = 0)\n");
		return 0;
	}

	src->source_size = fread(src->source_str, 1, MAX_SOURCE_SIZE, fp);
	if (src->source_size == 0) {
		printf("Kernel is empty (source_str = 0)\n");
		return 0;
	}

	// Close the .cl file and return
	fclose(fp);
	return 1;
}

cl_kernel createKernel(cl_context context, cl_device_id device_id, char* kernel_name, const char** src, const size_t* size) {
	cl_kernel kernel = NULL;
	cl_program program = NULL;
	cl_int err_num = NULL;

	// Create the program with Kernel source
	printf("Creating the Kernel for command: %s\n", kernel_name);
	program = clCreateProgramWithSource(context, 1, src, size, &err_num);
	if (!errorCheck(err_num)) return NULL;
	// Build the program
	err_num = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
	//Print logs if build failed
	if (err_num == CL_BUILD_PROGRAM_FAILURE) {
		size_t log_size = 0;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		// Allocate memory for the log
		char* log = (char*)malloc(log_size);
		// Get the log
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		// Print the log
		printf("LOG: %s\n", log);
		// Free after print
		free(log);
	}
	// Create the actual Kernel
	kernel = clCreateKernel(program, kernel_name, &err_num);
	if (!errorCheck(err_num)) return NULL;
	clReleaseProgram(program);
	// Kernel created, return it
	return kernel;
}

cl_device_id getGPUDevice() {
	/*SOURCE:
	* "List OpenCL platforms and devices " - https://gist.github.com/courtneyfaulkner/7919509
	*/
	// For some reason my OpenCL platform order changed, so I had to make this loop to select the NVIDIA GPU
	cl_uint platform_num, device_num;
	cl_platform_id* platforms;
	cl_device_id device;
	size_t val_size;
	char* val;

	// Get platforms
	clGetPlatformIDs(0, NULL, &platform_num);
	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * platform_num);
	clGetPlatformIDs(platform_num, platforms, NULL);

	for (int i = 0; i < platform_num; i++) {
		// Get device from current platform (I only have 1 GPU device on each platform)
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 1, &device, &device_num);
		// Check that current device is NVIDIA a GPU
		clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &val_size);
		val = (char*)malloc(val_size);
		clGetDeviceInfo(device, CL_DEVICE_NAME, val_size, val, NULL);
		if (!strcmp(val, "NVIDIA GeForce GTX 1070")) {
			break;
		}
	}
	// Free the mallocs
	free(val);
	free(platforms);
	// Print device info and return result
	printDeviceInfo(device);
	return device;
}

void printDeviceInfo(cl_device_id device) {
	/* Sources:
	* "List OpenCL platforms and devices" - https://gist.github.com/courtneyfaulkner/7919509
	*/
	char* info;
	cl_device_type dev_type = NULL;
	size_t info_size;

	printf("Info about the device being used for the Assignment:\n");
	// Print device name
	clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &info_size);
	info = (char*)malloc(info_size);
	clGetDeviceInfo(device, CL_DEVICE_NAME, info_size, info, NULL);
	printf("Name: %s\n", info);
	free(info);

	// Print device type
	clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(dev_type), &dev_type, NULL);
	if (dev_type == CL_DEVICE_TYPE_GPU) {
		printf("Type: GPU\n");
	}
	else if (dev_type == CL_DEVICE_TYPE_CPU) {
		printf("Type: CPU\n");
	}

	// Print OpenCL version
	clGetDeviceInfo(device, CL_DEVICE_VERSION, 0, NULL, &info_size);
	info = (char*)malloc(info_size);
	clGetDeviceInfo(device, CL_DEVICE_VERSION, info_size, info, NULL);
	printf("OpenCL Version: %s\n", info);
	free(info);

	// Print local mem type
	cl_device_local_mem_type mem_type = NULL;
	clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_TYPE, sizeof(cl_device_local_mem_type), &mem_type, NULL);
	if (mem_type == CL_GLOBAL) {
		printf("Local Device Memory Type: Global\n");
	}
	else {
		printf("Local Device Memory Type: Local\n");
	}

	// Print local mem size
	cl_ulong mem_size = NULL;
	clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &mem_size, NULL);
	printf("Local Device Memory Size: %dkB\n", (unsigned int)mem_size / 1024);

	// Print device max compute units
	cl_uint max_comp_units = NULL;
	clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &max_comp_units, NULL);
	printf("Device Maximum Compute Units: %d\n", max_comp_units);

	// Print device max clock frequency
	cl_uint clock_freq = NULL;
	clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &clock_freq, NULL);
	printf("Device Maximum Clock Frequency: %uMHz\n", clock_freq);

	// Print device maximum constant buffer size
	cl_ulong buff_size = NULL;
	clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &buff_size, NULL);
	printf("Device Maximum Buffer Size: %ukB\n", (unsigned int)buff_size / 1024);

	// Print device max work group size
	size_t group_size = NULL;
	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &group_size, NULL);
	printf("Device Maximum Work Group Size: %u\n", group_size);

	// Print device max work item sizes
	size_t item_sizes[3];
	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(item_sizes), &item_sizes, NULL);
	printf("Maximum Device Work Item Sizes: %u, %u, %u\n", item_sizes[0], item_sizes[1], item_sizes[2]);

	printf("\n");
}

cl_image_format getRGBAImageFormat() {
	cl_image_format format;
	format.image_channel_order = CL_RGBA; // The image was read as RGBA
	format.image_channel_data_type = CL_UNORM_INT8;
	return format;
}

cl_image_format getGrayImageFormat() {
	cl_image_format format_gray;
	format_gray.image_channel_order = CL_LUMINANCE; // The image was is in grayscale
	format_gray.image_channel_data_type = CL_UNORM_INT8;
	return format_gray;
}

std::vector<unsigned char> executeImageKernel(cl_command_queue cmd_q, cl_kernel kernel, unsigned new_w, unsigned new_h, cl_mem out_cl) {
	cl_event event;
	size_t global_work_size[] = { new_w, new_h };
	size_t local_work_size[] = { 1, 1 };
	size_t origin[3] = { 0, 0, 0 };
	size_t region[3] = { new_w, new_h, 1 };
	std::vector<unsigned char> out(new_w * new_h);
	int err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_work_size, local_work_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return out;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	// Initialize profiling parameters
	cl_ulong opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	double milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);
	// Get the resulting grayscaled image
	err_num = clEnqueueReadImage(cmd_q, out_cl, CL_TRUE, origin, region, 0, 0, &out[0], 0, NULL, NULL);
	if (!errorCheck(err_num)) return out;
	// Free the event
	err_num = clReleaseEvent(event);
	if (!errorCheck(err_num)) return out;
	// Return result
	return out;
}

std::vector<unsigned char> executeBufferKernel(cl_command_queue cmd_q, cl_kernel kernel, size_t global_size[], size_t local_size[], unsigned new_w, unsigned new_h, cl_mem out_cl) {
	// initialize required variables
	cl_event event;
	std::vector<unsigned char> out(new_w * new_h);
	// Start the kernel execution
	int err_num = clEnqueueNDRangeKernel(cmd_q, kernel, 2, NULL, global_size, local_size, 0, NULL, &event);
	if (!errorCheck(err_num)) return out;
	// Wait for execution to finish
	clWaitForEvents(1, &event);
	// Initialize profiling parameters
	cl_ulong opencl_start = 0, opencl_end = 0;
	// Calculate execution time
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_START, sizeof(cl_ulong), &opencl_start, NULL);
	clGetEventProfilingInfo(event, CL_PROFILING_COMMAND_END, sizeof(cl_ulong), &opencl_end, NULL);
	double milliseconds = (cl_double)(opencl_end - opencl_start) * (cl_double)(1e-06);
	printf("Kernel execution done, took %f milliseconds\n", milliseconds);
	// Get the resulting  image
	err_num = clEnqueueReadBuffer(cmd_q, out_cl, CL_TRUE, 0, new_w * new_h * sizeof(unsigned char), &out[0], 0, NULL, NULL);
	if (!errorCheck(err_num)) return out;
	// Free the event
	err_num = clReleaseEvent(event);
	if (!errorCheck(err_num)) return out;
	// Return result
	return out;
}