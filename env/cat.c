#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  FILE *fp;
  char ch;
  // if (argc == 1) {
  //   return 0;
  // }
  int i = 1;
  char buffer[1024];
  int nread;
  // if exist input, read from stdin
  // int fd = fileno(stdin);
  // int flags = fcntl(fd, F_GETFL, 0);
  // fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  if (argc > 1) {
  } else {
    while ((nread = read(STDIN_FILENO, buffer, sizeof(10))) > 0) {
      if (write(STDOUT_FILENO, buffer, nread) != nread) {
        perror("write error");
        return 1;
      }
    }
  }
  while (argv[i] != NULL) {
    fp = fopen(argv[i], "r");
    // printf("%s\n",argv[1]);
    if (fp == NULL) {
      perror("cat");
      i++;
      continue;
    }
    while ((ch = fgetc(fp)) != EOF) {
      putchar(ch);
    }
    fclose(fp);
    i++;
  }
  return 0;
}
