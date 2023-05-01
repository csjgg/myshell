#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Please input right commend\n");
    return 1;
  }
  if (strcmp(argv[1], "-s") == 0) {
    if (symlink(argv[2], argv[3]) == 0) {
      printf("Symbolic link created: %s -> %s\n", argv[2], argv[3]);
    } else {
      perror("Failed to create symbolic link");
      return 1;
    }
  } else {
    if (link(argv[1], argv[2]) == 0) {
      printf("Hard link created: %s -> %s\n", argv[1], argv[2]);
    } else {
      perror("Failed to create hard link");
      return 1;
    }
  }
  return 0;
}
