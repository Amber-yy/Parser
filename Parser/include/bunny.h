#ifndef BUNNY
#define BUNNY

struct image;
image *newImage();
void *delImage(image *img);
void *getImage(image *img,const char *src);

#endif