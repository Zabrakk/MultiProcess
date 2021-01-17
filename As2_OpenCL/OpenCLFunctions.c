#include "OpenCLFunctions.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/** Most of this file uses this source:
*** HelloWord Report.pdf from the course
**/

// OpenCL include
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif // __APPLE__

#define MAX_SOURCE_SIZE (0x001000)

int errorCheck(cl_int err_num) {
	if (err_num == CL_DEVICE_NOT_FOUND) {
		printf("CL_DEVICE_NOT_FOUND!\n");
		return 0;
	}
	else if (err_num == CL_INVALID_CONTEXT) {
		printf("CL_INVALID_CONTEXT!\n");
		return 0;
	}
	else if (err_num != CL_SUCCESS) {
		printf("An OpenCL error occured!\n");
		return 0;
	}
	return 1;
}


kernel_source loadKernel(char file_name[]) {
	FILE* fp;
	kernel_source src;
	src.ok = 1; // Set ok to true

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
	return src;
}

cl_device_id getGPUDevice() {
	char* info;
	size_t info_size;
	cl_int err_num;
	cl_uint num_of_platforms = NULL, num_of_devices = NULL;
	cl_platform_id *platforms = NULL, pid = NULL;
	cl_device_id *devices = NULL, device = NULL;

	// Get num of platforms
	//err_num = clGetPlatformIDs(1, NULL, num_of_platforms);
	err_num = clGetPlatformIDs(1, &pid, &num_of_platforms);
	if (num_of_platforms == 0 || !errorCheck(err_num)) {
		printf("No OpenCL platforms found!\n");
		return NULL;
	}
	// Get all platforms
	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * num_of_platforms);
	if (platforms == NULL) {
		printf("No OpenCL platforms found!\n");
		return NULL;
	}
	// Get platform IDs
	err_num = clGetPlatformIDs(num_of_platforms, platforms, NULL);
	if (!errorCheck(err_num)) return NULL;
	// Loop through all the platforms
	for (int i = 0; i < num_of_platforms; i++) {

		// Get num of GPU devices on current platform
		err_num = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_of_devices);
		if (!errorCheck(err_num)) return NULL;

		if (num_of_devices == 0) continue;
		// Get all GPU devices
		devices = (cl_device_id*)malloc(sizeof(cl_device_id) * num_of_devices);
		if (devices == NULL) continue;
		// Get their IDs
		err_num = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, num_of_devices, devices, NULL);
		if (&devices[0] == NULL || !errorCheck(err_num)) continue;
		
		// A GPU was found, since no continue was called

		//Print device info
		printf("Using the following GPU:\n");

		err_num = clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &info_size);
		if (!errorCheck(err_num)) continue;
		info = (char*)malloc(info_size);
		err_num = clGetDeviceInfo(devices[0], CL_DEVICE_NAME, info_size, info, NULL);
		if (!errorCheck(err_num)) continue;
		printf("Name: %s\n", info);
		free(info);

		clGetDeviceInfo(devices[0], CL_DEVICE_VERSION, 0, NULL, &info_size);
		info = (char*)malloc(info_size);
		clGetDeviceInfo(devices[0], CL_DEVICE_VERSION, info_size, info, NULL);
		printf("OpenCL Version: %s\n", info);
		free(info);

		clGetDeviceInfo(devices[0], CL_DRIVER_VERSION, 0, NULL, &info_size);
		info = (char*)malloc(info_size);
		clGetDeviceInfo(devices[0], CL_DRIVER_VERSION, info_size, info, NULL);
		printf("Driver software version: %s\n", info);
		free(info);

		// Return the device
		device = devices[0];
		return device;		
	}
	// No GPU found
	return NULL;
}

cl_context getContext(cl_device_id device_id) {
	cl_context context = NULL;
	cl_uint err_num = NULL;
	
	// Create OpenCL context for the given device
	context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &err_num);
	if (!errorCheck(err_num)) return NULL;
	// No error occured, return the context
	return context;
}

cl_command_queue getCommandQueue(cl_context context, cl_device_id device_id) {
	cl_int err_num = NULL;
	cl_command_queue cmd_q = NULL;

	// Create OpenCL command queue for the given parameters
	cmd_q = clCreateCommandQueue(context, device_id, 0, &err_num);
	if (!errorCheck(err_num)) return NULL;
	// No error occured, return the command queue
	return cmd_q;
}