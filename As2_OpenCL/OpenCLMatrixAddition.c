// OpenCL include
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif // __APPLE__
#include <stdio.h>
// Custom includes
#include "MatrixFunctions.h"
#include "OpenCLFunctions.h"

/*
void pprint(kernel_source* src) {
	fprintf("%s\n", src->source_size);
}
*/

#define MEM_SIZE (128)

int main() {
	// Based on the HelloWorld Report provided from the course
	cl_device_id device_id = NULL;
	cl_context context = NULL;
	cl_command_queue cmd_q = NULL;
	cl_mem mem_obj = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;

	// Load the kernel
	char file_name[] = "add_matrix.cl";
	kernel_source src = loadKernel(file_name);
	if (src.ok == 0) return 1;

	// Get a GPU ID
	device_id = getGPUDevice();
	if (device_id == NULL) return 1;
	// Create a context for the GPU
	context = getContext(device_id);
	if (context == NULL) return 1;
	// Create a command queue
	cmd_q = getCommandQueue(context, device_id);
	if (cmd_q == NULL) return 1;
	// Create memory buffer
	mem_obj = getMemoryBuffer(context, MEM_SIZE);
	if (mem_obj == NULL) return 1;
	// Create Kernel Program
	program = getProgram(context, device_id, (const char**)&src.source_str, (const size_t*)&src.source_size);

	printf("\n\n");
	getchar();
	// Program finished
	return 0;
}