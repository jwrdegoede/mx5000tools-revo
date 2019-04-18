#ifndef MX5000IMAGE_H
#define MX5000IMAGE_H


void  print_mx5000image(const unsigned char *image, int len, 
			int width, int height);

unsigned char *pbm_to_mx5000image(char *filename, int *width, int *height);


#endif /*  MX5000IMAGE_H */
