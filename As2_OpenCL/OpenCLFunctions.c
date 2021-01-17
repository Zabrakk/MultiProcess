#include "OpenCLFunctions.h"
#include <stdio.h>
#include <stdlib.h>

// OpenCL include
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif // __APPLE__

#define MAX_SOURCE_SIZE (0x001000)


kernel_source LoadKernel(char file_name[]) {
	/* Sources:
	* HelloWord Report.pdf from the course
	*/
	// Define variables
	FILE* fp;
	kernel_source src;
	src.ok = 1; // Set ok

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

cl_context getGPUContext() {
	char* info;
	size_t info_size;
	cl_int err_num;
	cl_context context;
	cl_uint num_of_platforms = NULL, num_of_devices = NULL;
	cl_platform_id* platforms = NULL;
	cl_device_id* devices;

	cl_platform_id pid;
	cl_device_id did;

	// Get num of platforms
	//err_num = clGetPlatformIDs(1, NULL, num_of_platforms);
	err_num = clGetPlatformIDs(1, &pid, &num_of_platforms);
	if (num_of_platforms == 0) {
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
	clGetPlatformIDs(num_of_platforms, platforms, NULL);
	// Loop through all the platforms
	for (int i = 0; i < num_of_platforms; i++) {

		// Get num of GPU devices on current platform
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, 0, NULL, &num_of_devices);

		if (num_of_devices == 0) continue;
		// Get all GPU devices
		devices = (cl_device_id*)malloc(sizeof(cl_device_id) * num_of_devices);
		if (devices == NULL) continue;
		// Get their IDs
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_GPU, num_of_devices, devices, NULL);
		// Loop through the devices and return the first one <-- REMOVE THE LOOP
		for (int j = 0; j < num_of_devices; j++) {
			if (&devices[i] == NULL) return NULL;

			//Print device info
			printf("Using the following GPU:\n");

			clGetDeviceInfo(devices[j], CL_DEVICE_NAME, 0, NULL, &info_size);
			info = (char*)malloc(info_size);
			clGetDeviceInfo(devices[j], CL_DEVICE_NAME, info_size, info, NULL);
			printf("%s\n", info);
			free(info);

			clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &info_size);
			info = (char*)malloc(info_size);
			clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, info_size, info, NULL);
			printf("OpenCL Version: %s\n", info);
			free(info);

			clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &info_size);
			info = (char*)malloc(info_size);
			clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, info_size, info, NULL);
			printf("Driver software version: %s\n", info);
			free(info);

			// Create OpenCl context for this device and return it
			context = clCreateContext(NULL, 1, &devices[i], NULL, NULL, &err_num);
			return context;
		}
	}
	return NULL;
}