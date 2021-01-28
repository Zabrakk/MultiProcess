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
	// Kernel created, return it
	printf("Kernel created\n");
	return kernel;
}

void printDeviceInfo(cl_device_id device) {
	/* Sources:
	* "List OpenCL platforms and devices" - https://gist.github.com/courtneyfaulkner/7919509
	*/
	char* info;
	cl_device_type type = NULL;
	size_t info_size;

	printf("Info about the device being used for the Assignment:\n");
	// Print device name
	clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &info_size);
	info = (char*)malloc(info_size);
	clGetDeviceInfo(device, CL_DEVICE_NAME, info_size, info, NULL);
	printf("Name: %s\n", info);
	free(info);

	// Print device type
	clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
	//type = (int)malloc(info_size);
	//clGetDeviceInfo(device, CL_DEVICE_TYPE, info_size, type, NULL);
	if (type == CL_DEVICE_TYPE_GPU) {
		printf("Type: GPU\n");
	}
	else if (type == CL_DEVICE_TYPE_CPU) {
		printf("Type: CPU\n");
	}

	// Print OpenCL version
	clGetDeviceInfo(device, CL_DEVICE_VERSION, 0, NULL, &info_size);
	info = (char*)malloc(info_size);
	clGetDeviceInfo(device, CL_DEVICE_VERSION, info_size, info, NULL);
	printf("OpenCL Version: %s\n", info);
	free(info);
	printf("\n");
}