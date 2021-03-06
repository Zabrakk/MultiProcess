#include "OpenCLFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

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
	default:
		printf("An unexpected OpenCL error occured! Code was %d\n", err_num);
	}
	printf("Enter something to exit: ");
	getchar();
	return 0;
}

kernel_source loadKernel(char file_name[]) {
	FILE* fp;
	kernel_source src;
	src.ok = 1; // Set ok to true
	printf("Loading OpenCL Kernel\n");

	// Load source code, and store to kernel_source struct
	fopen_s(&fp, file_name, "r");
	if (!fp) {
		src.ok = 0;
		printf("Failed to load kernel, .cl file not found!\n");
		return src;
	}

	src.source_str = (char*)calloc(MAX_SOURCE_SIZE, 1);
	if (src.source_str == 0) {
		src.ok = 0;
		printf("Kernel is empty (source_str = 0)\n");
		return src;
	}

	src.source_size = fread(src.source_str, 1, MAX_SOURCE_SIZE, fp);
	if (src.source_size == 0) {
		src.ok = 0;
		printf("Kernel is empty (source_str = 0)\n");
		return src;
	}

	// Close the .cl file and return
	fclose(fp);
	printf("OpenCL Kernel loaded\n\n");
	return src;
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
	kernel = clCreateKernel(program, kernel_name, &err_num);
	if (!errorCheck(err_num)) return NULL;
	clReleaseProgram(program);
	// Kernel created, return it
	printf("Kernel created\n"); // Should keep this here?
	return kernel;
}

void printCharInfo(cl_device_id device, unsigned info_name) {
	char* info;

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
	} else {
		printf("Local Device Memory Type: Local\n");
	}
	
	// Print local mem size
	cl_ulong mem_size = NULL;
	clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &mem_size, NULL);
	printf("Local Device Memory Size: %dkB\n", (unsigned int)mem_size/1024);

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