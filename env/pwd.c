#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
int main(int argc, char** argv) {
  char* cwd;
  char* buff = (char*)malloc(100);
  cwd = getcwd(buff, 100);
  if (cwd == NULL)
    perror("pwd error");
  else
    printf("%s\n", cwd);
  free(buff);
  return 0;
}