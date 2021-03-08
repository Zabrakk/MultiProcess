__kernel void occlusion_fill(__global const unsigned char* cross, __global unsigned char* dst, unsigned int w, unsigned int h) {
	// Perform Occlusion Fill for the Cross checked image
	
	int x = get_global_id(0);
	int y = get_global_id(1);
	int coord = y*w + x;
	int nh_size = 150;
	int stop = 0;
	
	int current_value = cross[coord];
	if (current_value != 0) {
		dst[coord] = current_value;
	} else {
		for(int spread = 1; spread <= nh_size/2; spread++) {
			for (int y_nh = -spread; y_nh <= spread; y_nh++) {
				for (int x_nh = -spread; x_nh <= spread; x_nh++) {
					if (y_nh + y < 0 || y_nh + y >= h || x_nh + x < 0 || x_nh + x >= w || (y_nh == 0 && x_nh == 0)) {
						continue;
					}
					current_value = cross[(y + y_nh) * w + (x + x_nh)];
					if (current_value != 0) {
						break;
					}
				}
				if(current_value != 0) {
					break;
				}
			}
			if(current_value != 0) {
				break;
			}
		}
		dst[coord] = current_value;
	}
}