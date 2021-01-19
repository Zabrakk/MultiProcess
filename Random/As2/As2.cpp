#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <stdio.h>
#include <stdbool.h> // For booleans
#include <CL/cl.h>
// Custom header includes
#include "ErrorCheck.h"
#include "Context.h"
#include "CommandQueue.h"

int main() {
	// Based on the OpenCL Programming Guide from the Moodle References section, pages 46-48
	cl_context context = 0;
	cl_command_queue cmdQ = 0;
	cl_program program = 0;
	cl_device_id device = 0;
	cl_kernel kernel = 0;
	cl_mem memObj[3] = { 0, 0, 0 };
	cl_int errNum;

	// Create an OpenCL GPU context on first available platform
	context = create_GPU_context();
	if (context == NULL) return -1; // Exit on failure

	// Create a command queue for the first available device
	cmdQ = create_command_queue(context, &device);
	if (cmdQ == NULL) {
		//Cleanup(context, cmdQ, program, kernel, memObj); // Cleanup??
		return -1;
	}

	printf("Done! Type something to exit: ");
	getchar();
	return 0;
}