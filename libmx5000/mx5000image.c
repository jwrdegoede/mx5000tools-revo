
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <netpbm/pbm.h>
#include "config.h"


void  print_mx5000image(const unsigned char *image, int len, int width, int height)
{
  char buf[width][height];
  int x=0, y=0, i=0;

  for (i=0;i<len*8;i++) {
    buf[x][y] = (image[i/8]>>(7-(i%8)))&1;
    y++;
    if (y%8 == 0) {
      x++;
      if (x >= width) {
	x = 0;
      } else {
	y -= 8;
      }
    }
    
  }
  for (y=0; y<height; y++) {
    for (x=0; x<width; x++) {
      
      if (buf[x][y])
	printf("X");
      else
	printf(" ");
    }
    printf("\n");
  }

}


unsigned char *pbm_to_mx5000image(char *filename, int *width, int *height)
{
  FILE *file = NULL;
  int x=0,y=0;
  bit **image = NULL;
  unsigned char *mx5000image = NULL;
  int readheight;
  int i;
  
  *width = *height = 0;

  file = fopen(filename, "r");

  if (!file) {
    perror("opening pbm file");
    return NULL;
  }

  image = pbm_readpbm(file, width, &readheight);


  if (!image) {
    goto exit1;
  }

  *height = readheight;
  if (*height % 8) {
    *height += 8-(readheight%8);
  }


  mx5000image = malloc(*width * *height);

  if (mx5000image == NULL) {
    goto exit2;
  }


  memset(mx5000image, 0, *width * *height);

  for (i=0;i<(*width * readheight)*8;i++) {
    if (y < readheight && image[y][x])
      mx5000image[i/8] |= 1<<(7-i%8);

    y++;
    if (y%8 == 0) {
      x++;
      if (x >= *width) {
	x = 0;
      } else {
	y -= 8;
      }
    }
  }

 exit2:

  pbm_freearray(image, readheight);
 exit1:
  fclose(file);

  printf("w: %d h: %d\n", *width, *height);


  return mx5000image;

}
