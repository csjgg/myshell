#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
  if (argc != 2) {
    printf("please input dirname\n");
    return 1;
  }

  int res = mkdir(argv[1], 0777);
  if (res == -1) {
    perror("mkdir failed");
    return 1;
  }
  return 0;
}