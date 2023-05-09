#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#define BUFFER_SIZE 1024

int IS_DIR(char *path);
int cp_file(char *src, char *dest);

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Please input right commend\n");
    return 0;
  }
  if (argc > 3) {    // mutiple files
    if (IS_DIR(argv[argc - 1])) {
      for (int i = 1; i < argc - 1; i++) {  // copy each file
        char name[20];
        char oldpath[50];
        char newpath[50];
        strcpy(oldpath, argv[i]);
        strcpy(newpath, argv[argc - 1]);
        char *new = strtok(argv[i], "/");
        do {
          strcpy(name, new);
          new = strtok(NULL, "/");
        } while (new != NULL);
        strcat(newpath, "/");
        strcat(newpath, name);
        cp_file(oldpath, newpath);
      }
    } else {
      printf("%s is not a directory\n", argv[argc - 1]);
    }
  } else {  // single file
    if (IS_DIR(argv[2])) {
      char name[20];
      char oldpath[50];
      char newpath[50];
      strcpy(oldpath, argv[1]);
      strcpy(newpath, argv[2]);
      char *new = strtok(argv[1], "/");
      do {
        strcpy(name, new);
        new = strtok(NULL, "/");
      } while (new != NULL);
      strcat(newpath, "/");
      strcat(newpath, name);
      cp_file(oldpath, newpath);
    } else {
      cp_file(argv[1], argv[2]);
    }
  }
  return 0;
}

int IS_DIR(char *path) {
  struct stat st;
  if (stat(path, &st) != 0) {
    return 0;
  }
  return S_ISDIR(st.st_mode);
}

int cp_file(char *src, char *dest) {
  FILE *src_file, *dest_file;

  src_file = fopen(src, "r");  // open source file
  if (!src_file) {
    printf("Failed to open source file.\n");
    exit(1);
  }

  dest_file = fopen(dest, "w");  // open destination file
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