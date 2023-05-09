#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int execute(char* argv[], char* delim, int i,
            int flag);  // i is used to show the begin
// flag 0 means no commend
int main(int argc, char* argv[]) {
  if (argc >= 2 && (argv[1][0] == '-' && argv[1][1] == 'd')) {  // xargs -d
    if (argc < 4) {
      execute(argv, argv[2], 2, 0);
    } else {
      execute(argv, argv[2], 2, 1);
    }
  } else {  // normal xargs
    if (argc < 2) {
      execute(argv, " ", 1, 0);
    } else {
      execute(argv, " ", 1, 1);
    }
  }
  return 0;
}

int execute(char* argv[], char* delim, int i, int flag) {
  char* commend[30] = {NULL};
  char buffer[1024];
  int m = i;
  // // set no block
  // int fd = fileno(stdin);
  // int flags = fcntl(fd, F_GETFL, 0);
  // fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  while (argv[i] != NULL) {
    commend[i - m] = (char*)malloc(strlen(argv[i]) + 2);
    strcpy(commend[i - m], argv[i]);
    i++;
  }
  i -= m;
  while (read(STDIN_FILENO, buffer, sizeof(buffer))) {
    char* tmp = strtok(buffer, delim);
    // write into commend
    while (tmp != NULL) {
      commend[i] = (char*)malloc(strlen(tmp) + 4);
      strcpy(commend[i], tmp);
      commend[i][strcspn(commend[i], "\n")] = '\0';
      i++;
      tmp = strtok(NULL, delim);
    }
  }
  commend[i] = NULL;
  // execute commend
  int afd = fork();
  if (afd < 0) {
    perror("fork error");
  } else if (afd == 0) {
    if (flag == 0) {
      char* newcommend[30] = {NULL};
      newcommend[0] = "/bin/echo";
      int j = 1;
      while (commend[j - 1] != NULL) {
        newcommend[j] = commend[j - 1];
        j++;
      }
      newcommend[j] = NULL;
      execvp("/bin/echo", newcommend);
    } else {
      execvp(commend[0], commend);
    }
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