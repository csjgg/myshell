#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
  FILE *src_file, *dest_file;

  if (argc != 3) {
    printf("Please input right commend\n");
    exit(1);
  }

  src_file = fopen(argv[1], "r");  // open source file
  if (!src_file) {
    printf("Failed to open source file.\n");
    exit(1);
  }

  dest_file = fopen(argv[2], "w");  // open destination file
  if (!dest_file) {
    printf("Failed to create destination file.\n");
    fclose(src_file);
    exit(1);
  }

  char ch;  // copy file character by character
  while ((ch = fgetc(src_file)) != EOF) {
    fputc(ch, dest_file);
  }

  fclose(src_file);
  fclose(dest_file);

  return 0;
}
