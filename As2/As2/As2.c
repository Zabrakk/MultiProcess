#define CL_USE_DEPRECATED_OPENCL_1_2_APIS

#include <stdio.h>
#include <CL/cl.h>

int main() {
	
	int err; // Store Error Code returned by the API here
	cl_device_id device_id;
	cl_uint maxComputeUnits;

	err = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 1, &device_id, NULL);
	if (err != CL_SUCCESS) {
		printf("FUCK");
		getchar();
		return -1;
	}
	err = clGetDeviceInfo(device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(maxComputeUnits), &maxComputeUnits, NULL);
	if (err != CL_SUCCESS) return -1;

	printf("Parallel Compute Units: %d\n", maxComputeUnits);
	getchar();
	return 0;
}