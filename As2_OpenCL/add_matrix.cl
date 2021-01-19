__kernel void add_matrix(__global const int* m1, __global const int* m2, __global int* result) {
	int id = get_global_id(0);
	result[id] = m1[id] + m2[id];
}