#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int IS_DIR(char *path);

int main(int argc, char *argv[]) {
  if (argc != 3) {
    printf("Prease input right commend\n");
    return 0;
  }
  if (IS_DIR(argv[2])) {
    char name[20];
    char oldpath[50];
    strcpy(oldpath, argv[1]);
    char *new = strtok(argv[1], "/");
    do {
      strcpy(name, new);
      new = strtok(NULL, "/");
    } while (new != NULL);
    strcat(argv[2], "/");
    strcat(argv[2], name);
    if (rename(argv[1], argv[2]) != 0) {
      perror("mv");
    }
  } else {
    if (rename(argv[1], argv[2]) != 0) {
      perror("mv");
      return 0;
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