#include "Context.h"
#include "ErrorCheck.h"
#include <stdio.h>
#include <string.h>
#include <CL/cl.h>

/**
* Context obtaining
*/

cl_context create_platform_context(char type[]) {
	// Based on the OpenCL Programming Guide from the Moodle References section, pages 49-50
	/**
	CURRENTLY FAILS IF type = "CPU", most likely because the selected platform is a GPU
	**/
	cl_int errNum;
	cl_uint numOfPlatforms;
	cl_platform_id firstPlatformId;
	cl_context context = NULL;


	// Select first platform, and get its Id
	errNum = clGetPlatformIDs(1, &firstPlatformId, &numOfPlatforms);
	if (!error_check(errNum) || numOfPlatforms <= 0) {
		printf("Failed to find suitable OpenCL platform!\n");
		return NULL;
	}

	// Create an OpenCL context for the selected platform type
	cl_context_properties properties[] = {
		CL_CONTEXT_PLATFORM,
		(cl_context_properties)firstPlatformId,
		0
	};
	if (strcmp(type, "CPU") == 0) {
		context = clCreateContextFromType(properties, CL_DEVICE_TYPE_CPU, NULL, NULL, &errNum);
	}
	else if (strcmp(type, "GPU") == 0) {
		context = clCreateContextFromType(properties, CL_DEVICE_TYPE_GPU, NULL, NULL, &errNum);
	}
	else {
		printf("%s is an invalid platform type!\n", type);
		return NULL;
	}
	if (!error_check(errNum)) {
		printf("Failed to create OpenCL %s context!\n", type);
		return NULL;
	}

	// The platform context is now created, return it
	return context;
}


cl_context create_GPU_context() {
	return create_platform_context("GPU");
}


cl_context create_CPU_context() {
	return create_platform_context("CPU");
}