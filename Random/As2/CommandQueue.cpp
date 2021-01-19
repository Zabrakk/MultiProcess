#include "CommandQueue.h"
#include "ErrorCheck.h"
#include <stdio.h>
#include <CL/cl.h>

cl_command_queue create_command_queue(cl_context context, cl_device_id* device) {
	// Based on the OpenCL Programming Guide from the Moodle References section, pages 51-52
	cl_int errNum;
	cl_device_id* devices;
	cl_command_queue cmdQ = NULL;
	size_t deviceBuffSize = -1;

	// Obtain device buffer size
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &deviceBuffSize);
	if (!error_check(errNum)) {
		printf("Failed to get context info\n");
		return NULL;
	}
	else if (deviceBuffSize <= 0) {
		printf("No device available (deviceBuffSize <= 0)\n");
		return NULL;
	}

	// Allocate memory for the device's buffer
	devices = new cl_device_id[deviceBuffSize / sizeof(cl_device_id)];
	errNum = clGetContextInfo(context, CL_CONTEXT_DEVICES, deviceBuffSize, devices, NULL);
	if (!error_check(errNum)) {
		printf("Failed to obtaine device IDs\n");
		return NULL;
	}

	// Select a device. CURRENTLY SELECTS THE FIRST AVAILALE ONE!! pp.52
	cmdQ = clCreateCommandQueue(context, devices[0], 0, NULL);
	if (cmdQ == NULL) {
		printf("Failed to create a command cue for the device\n");
		return NULL;
	}
	*device = devices[0];
	delete[] devices;
	return cmdQ;
}