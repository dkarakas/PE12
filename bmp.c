#include <stdlib.h>
#include <stdio.h>

#include "bmp.h"

/* check whether a header is valid
 * assume that header has been read from fptr
 * the position of the indicator of fptr is not certain
 * could be at the beginning of the file, end of the file or 
 * anywhere in the file
 * note that the check is only for this exercise/assignment
 * in general, the format is more complicated
 */

int Is_BMP_Header_Valid(BMP_Header* header, FILE *fptr) {
  // Make sure this is a BMP file
  if (header->type != 0x4d42) {
     return FALSE;
  }
  // skip the two unused reserved fields

  // check the offset from beginning of file to image data
  // essentially the size of the BMP header
  // BMP_HEADER_SIZE for this exercise/assignment
  if (header->offset != BMP_HEADER_SIZE) {
     return FALSE;
  }
      
  // check the DIB header size == DIB_HEADER_SIZE
  // For this exercise/assignment
  if (header->DIB_header_size != DIB_HEADER_SIZE) {
     return FALSE;
  }

  // Make sure there is only one image plane
  if (header->planes != 1) {
    return FALSE;
  }
  // Make sure there is no compression
  if (header->compression != 0) {
    return FALSE;
  }

  // skip the test for xresolution, yresolution

  // ncolours and importantcolours should be 0
  if (header->ncolours != 0) {
    return FALSE;
  }
  if (header->importantcolours != 0) {
    return FALSE;
  }
  
  // Make sure we are getting 24 bits per pixel
  // or 16 bits per pixel
  // only for this assignment
  if (header->bits != 24 && header->bits != 16) {
    return FALSE;
  }

  // fill in extra to check for file size, image size
  // based on bits, width, and height

  //set at the beggining
  fseek(fptr,0,SEEK_END);
  unsigned long int sizeFile = ftell(fptr);

  if(sizeFile != header->size){
    fprintf(stderr,"Not good header size\n");
    return FALSE;
  }
  if((sizeFile-54) != header->imagesize){
    fprintf(stderr,"Not good header image size\n");
    return FALSE;
  }
  

  //find need padding
  int bytePerPixel = header->bits/8;
  int padNeed = (header->width*bytePerPixel)%4;
  
  if(padNeed != 0){
    padNeed = 4 - padNeed;
  }

  if(header->imagesize != (header->height*((header->width*bytePerPixel)+padNeed))){
    fprintf(stderr,"Not good header image size//\n");
    return FALSE;
  }

  return TRUE;
}

/* The input argument is the source file pointer. 
 * The function returns an address to a dynamically allocated BMP_Image only 
 * if the file * contains a valid image file 
 * Otherwise, return NULL
 * If the function cannot get the necessary memory to store the image, also 
 * return NULL
 * Any error messages should be printed to stderr
 */
BMP_Image *Read_BMP_Image(FILE* fptr) {

  // go to the beginning of the file
  fseek(fptr,0,SEEK_SET);

  BMP_Image *bmp_image = NULL;

  //Allocate memory for BMP_Image*;
  bmp_image = (BMP_Image*)malloc(sizeof(BMP_Image));
  bmp_image->data = NULL;
  if(bmp_image==NULL){
    fprintf(stderr,"error malloc");
    return NULL;
  }

  //Read the first 54 bytes of the source into the header
  int read = fread((&(bmp_image->header)),sizeof(BMP_Header),1,fptr);
  // if read successful, check validity of header
  if(read == 1){
    if(!Is_BMP_Header_Valid(&bmp_image->header,fptr)){
      fprintf(stderr,"Can't read the image from file1\n");
      Free_BMP_Image(bmp_image);
      return NULL;
    }
  }else{
    return NULL;
  }
  // Allocate memory for image data
  bmp_image->data = (unsigned char *)malloc(bmp_image->header.imagesize);
  if(bmp_image->data==NULL){
    fprintf(stderr,"error malloc bmp_image->data");
    return NULL;
  }

  // read in the image data
  fseek(fptr,sizeof(BMP_Header),SEEK_SET);
  read = fread(bmp_image->data,bmp_image->header.imagesize,1,fptr);
  if(read != 1){
     fprintf(stderr,"Can't read the image from file2\n");
     Free_BMP_Image(bmp_image);
     return NULL;
  }

   //find need padding
   int bytePerPixel = bmp_image->header.bits/8;
   int padNeed = (bmp_image->header.width*bytePerPixel)%4;
   if(padNeed != 0){
      padNeed = 4 - padNeed;
   } 

  char (*arrayOriginalPicture)[bmp_image->header.width*bytePerPixel+padNeed]= (char (*)[bmp_image->header.width*bytePerPixel+padNeed])(bmp_image->data); 
  int i,j;

  for(i=0; i<bmp_image->header.height;i++)
    for(j=0; j<(bmp_image->header.width*bytePerPixel+padNeed);j++)
        if(   bmp_image->header.width*bytePerPixel <= j  ){
          arrayOriginalPicture[i][j] = 0;
        }
  return bmp_image;
}

/* The input arguments are the destination file pointer, BMP_Image *image.
 * The function write the header and image data into the destination file.
 * return TRUE if write is successful
 * FALSE otherwise
 */
int Write_BMP_Image(FILE* fptr, BMP_Image* image) 
{
   // go to the beginning of the file
   fseek(fptr,0,SEEK_SET);

   // write header
   int write = fwrite(&(image->header),sizeof(BMP_Header),1,fptr);
   if(write != 1){
     fprintf(stderr,"error printing file"); 
     return FALSE;
   }


   // write image data
   write = fwrite(image->data,image->header.imagesize,1,fptr);
   if(write != 1){
     fprintf(stderr,"error printing file"); 
     return FALSE;
   }

   return TRUE;
}

/* The input argument is the BMP_Image pointer. The function frees memory of 
 * the BMP_Image.
 */
void Free_BMP_Image(BMP_Image* image) {
  if(image!=NULL)
    if(image->data!=NULL)
      free(image->data);
  if(image!=NULL)
    free(image);
  return;
}

// Given a BMP_Image, create a new image that is a reflection 
// of the given image
// It could be a horizontal reflection (with the vertical mirror 
// being placed at the center of the image) 
// It could be a vertical reflection (with the horizontal mirror
// being placed at the center of the image)
// It could be a horizontal reflection followed by a vertical
// reflection (or equivalently, a vertical reflection followed by
// horizontal reflection).
// hrefl == 1 implies that a horizontal reflection should take place
// hrefl == 0 implies that a horizontal reflection should not take place
// vrefl == 1 implies that a vertical reflection should take place
// vrefl == 0 implies that a vertical reflection should not take place

BMP_Image *Reflect_BMP_Image(BMP_Image *image, int hrefl, int vrefl)
{
   //CREATING A NEW STRUCTURE FOR THE IMAGE AND COPYING THE HEADER.
   BMP_Image *t_image = NULL;
   if(image != NULL){
     //ALLOC SPACE FOR THE NEW STRUCTURE
     t_image = (BMP_Image *)malloc(sizeof(BMP_Image));
     if(t_image == NULL){
       return NULL;
     }
     t_image->data = NULL;

     t_image->data = (unsigned char *)malloc(image->header.imagesize);
     if(t_image->data==NULL){
       free(t_image);
       fprintf(stderr,"error malloc bmp_image->data");
       return NULL;
     }

     //COPYING THE HEADER INTO THE NEW STRUCTURE
     t_image->header = image->header;
   } 

   //find need padding
   int bytePerPixel = image->header.bits/8;
   int padNeed = (image->header.width*bytePerPixel)%4;
   if(padNeed != 0){
      padNeed = 4 - padNeed;
   } 

   char (*arrayPic)[image->header.width*bytePerPixel+padNeed] = (char (*)[image->header.width*bytePerPixel+padNeed])(t_image->data);
   char (*arrayOriginalPicture)[image->header.width*bytePerPixel+padNeed] = (char (*)[image->header.width*bytePerPixel+padNeed])(image->data); 

  int i,j;
  for(i=0; i<t_image->header.height;i++)
    for(j=0; j<(t_image->header.width*bytePerPixel+padNeed);j++)
        if(   t_image->header.width*bytePerPixel <= j  ){
          arrayPic[i][j] = 0;
        }


  int count;
  int move;
   if(hrefl == 1){
     for(i=0; i<image->header.height;i++){
      count = 1;
      move = 1;
       for(j=0; j < (image->header.width*bytePerPixel+padNeed);j++)
           if(   image->header.width*bytePerPixel > j  ){
                arrayPic[i][bytePerPixel*count-move] =  arrayOriginalPicture[i][image->header.width*bytePerPixel-1-j];
                move++;
                if(move == (bytePerPixel+1)){
                  count++;
                  move = 1;
                }
           }else{break;}
      }
     Free_BMP_Image(image);
     return t_image;
   }

   if(vrefl == 1){
     for(i=0; i<image->header.height;i++)
       for(j=0; j < image->header.width*bytePerPixel;j++)
                arrayPic[i][j] =  arrayOriginalPicture[image->header.height-i-1][j];
   Free_BMP_Image(image);
   return t_image;

   }
   return image;
}

