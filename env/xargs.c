#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int execute(char* argv[], char* delim,int i);  // i is used to show the begin

int main(int argc, char* argv[]) {

  if (argc > 2 && (argv[1][0] == '-'&&argv[1][1] == 'd')) {  // xargs -d
    char delim[10];
    int i = 2;
    if(argv[1][i]!='\0'){
      delim[i-2] = argv[1][i];
      i++;
    }
    delim[i-2] = '\0';
    execute(argv,delim,2);
  } else {  // normal xargs
    if (argc < 2) {
      printf("commend error\n");
      return 0;
    } else {
      execute(argv," ",1);
    }
  }
  return 0;
}

int execute(char* argv[], char* delim,int i) {
  char* commend[30] = {NULL};
  char buffer[1024];
  char dir[40] = "/home/csj/Desktop/shell/env/";
  int m = i;
  // set no block
  int fd = fileno(stdin);
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  while (argv[i] != NULL) {
    commend[i - m] = (char*)malloc(strlen(argv[i]) + 2);
    strcpy(commend[i - m], argv[i]);
    i++;
  }
  i-=m;
  read(STDIN_FILENO, buffer, sizeof(buffer));
  char* tmp = strtok(buffer, delim);
  while (tmp != NULL) {
    commend[i] = (char*)malloc(strlen(tmp) + 4);
    strcpy(commend[i], tmp);
    commend[i][strlen(commend[i])] = '\n';
    i++;
    tmp = strtok(NULL, delim);
  }
  commend[i] = NULL;
  int afd = fork();
  if (afd < 0) {
    perror("fork error");
  } else if (afd == 0) {
    strcat(dir, commend[0]);
    commend[0] = dir;
    execv(commend[0], commend);
    exit(1);
  } else if (afd > 0) {
    int status;
    wait(&status);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
      return 0;
    } else {
      printf("commend error\n");
      return 0;
    }
  }
  return 0;
}