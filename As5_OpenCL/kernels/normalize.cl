__kernel void normalize_img(__global const unsigned char* src, __global unsigned char* dst, unsigned int w, unsigned int min, unsigned int max) {
	
	int x = get_global_id(0);
	int y = get_global_id(1);
	int coord = y*w+x;
	
	dst[coord] = 255 * (src[coord]-min)/(max-min);
}