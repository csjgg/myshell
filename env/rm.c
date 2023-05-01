#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Please input the filename\n");
    return 0;
  }
  struct stat st;
  if (stat(argv[1], &st) == 0) {
    if (S_ISDIR(st.st_mode)) {
      if (remove(argv[1]) != 0) {
        perror("remove");
        exit(1);
      }
    } else if (S_ISREG(st.st_mode)) {
      if (unlink(argv[1]) != 0) {
        perror("unlink");
        exit(1);
      }
    }
  } else {
    printf("File not exist\n");
    return 0;
  }
  return 0;
}