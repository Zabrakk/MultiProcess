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

int main() {
	// Based on the HelloWorld Report provided from the course
	cl_context context = NULL;
	cl_device_id dev_id = NULL;
	cl_command_queue command_q = NULL;
	cl_mem mem_obj = NULL;
	cl_program program = NULL;
	cl_platform_id platform_id = NULL;
	cl_uint num_of_devices, num_of_platforms;
	cl_int ret;

	// Load the kernel
	char file_name[] = "add_matrix.cl";
	kernel_source src = LoadKernel(file_name);

	getGPUContext();

	printf("\n\n");
	getchar();
	// Program finished
	return 0;
}