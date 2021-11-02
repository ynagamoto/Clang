#include<stdio.h>
#include<stdlib.h>
#include<sys/stat.h>

int main() {
  FILE *fp;
  int file_size;
  
  if ((fp = fopen("./metal", "r")) == NULL) {
    perror("fopen");
    exit(1);
  }

  struct stat stbuf;
  if (fstat(fp->_fileno, &stbuf) == -1) {
    perror("fstat");
  }

  file_size = stbuf.st_size;
  
  char image_t[file_size];
  fread(image_t, sizeof(unsigned char), sizeof(image_t)/sizeof(image_t[0]), fp);

  printf("%s", image_t);

  fclose(fp);
   
  return 0;
}
