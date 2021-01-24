__kernel void to_grayscale(__global const unsigned char* img, __global unsigned char* result) {
	int id = get_global_id(0);
	result[id] = img[id*3]*0.299 + img[id*3+1]*0.587 + img[id*3+2]*0.114;
}