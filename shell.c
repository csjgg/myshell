
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define CMD_NUM 2
#include <readline/history.h>
#include <readline/readline.h>

int fgjob;

// function declaration
// internal command
char* cmd[] = {"cd", "exit"};
int cd(char** token);
int Exit(char** token);
int (*func[])(char**) = {&cd, &Exit};

// split the line into tokens
char** SplitLine(char* line, int* special, int* back);
// execute the command
int internalcom(char** token);
int ExecuteLine(char** token);
int ExecuteSpecialLine(char** token);
// output
void outputpipe(int TopPipe[]);
// relocate the output
int relocate(int status, char** tokens, char* filename,
             int toppipe[]);  // status 0: >  1: >> 2: <

// pipe
int my_pipe(char** pre_tokens, char** post_tokens, int toppipe[]);

void SetTheEnv(void);
void handler(int sig) {
  if (fgjob != 0) {
    killpg(fgjob, SIGINT);
  } else {
    printf("\n>>");
  }
}
int main() {
  char* line;
  char** tokens;
  int background = 0;  // 0: foreground 1: background
  int status = 1;      // 0: exit 1: continue
  int special = 0;     // 0: normal,do not have '<>|' 1: special have '<>|'
  int std_in = dup(STDIN_FILENO);
  int std_out = dup(STDOUT_FILENO);
  signal(SIGINT, handler);
  SetTheEnv();
  while (1) {
    fgjob = 0;
    line = readline(">>");
    if (line == NULL) break;
    add_history(line);
    tokens = SplitLine(line, &special, &background);
    if (tokens[0] == NULL) {
      free(line);
      line = NULL;
      continue;
    }
    status = internalcom(tokens);
    if (status != -1) {
      if (status == 0) break;
      if (status == 1) {
        free(line);
        line = NULL;
        continue;
      }
    }
    int fd = fork();
    if (fd < 0) {
      perror("fork error");
    } else if (fd == 0) {
      // setpgid(0, 0);
      if (special == 1) {
        status = ExecuteSpecialLine(tokens);
      } else {
        status = ExecuteLine(tokens);
      }
      exit(status);
    } else {
      if (background == 0) {
        fgjob = fd;
        // printf("%d\n",fd);
        waitpid(fd, &status, 0);
        // killpg(fd, SIGINT);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
          break;
        } else {
          // printf("OK\n");
        }
      } else {
        printf("(%d) %s\n", fd, tokens[0]);
      }
      background = 0;
      special = 0;
    }
    dup2(std_in, STDIN_FILENO);
    dup2(std_out, STDOUT_FILENO);
    free(line);
    line = NULL;
  }
  unsetenv("PATH");
  return 0;
}

void SetTheEnv(void) {
  char* new_path = "/home/csj/shell/myshell/env";
  char* old_path = getenv("PATH");

  if (old_path == NULL) {
    setenv("PATH", new_path, 1);
  } else {
    char* path = (char*)malloc(strlen(old_path) + strlen(new_path) + 2);
    sprintf(path, "%s:%s", new_path, old_path);
    setenv("PATH", path, 1);
    free(path);
  }
}

int cd(char** token) {
  if (token[1] == NULL) {
    chdir("/home");
  } else {
    if (chdir(token[1]) != 0) {
      perror("cd wrong");
    }
  }
  return 1;
}

int Exit(char** token) { return 0; }

char** SplitLine(char* line, int* special, int* back) {
  char** tokens = (char**)malloc(sizeof(char*) * 10);
  tokens[0] = strtok(line, " ");
  int i = 0;
  while (tokens[i] != NULL) {
    if (strcmp(tokens[i], "|") == 0 || strcmp(tokens[i], ">") == 0 ||
        strcmp(tokens[i], ">>") == 0 || strcmp(tokens[i], "<") == 0) {
      *special = 1;
    }
    if (strcmp(tokens[i], "&") == 0) {
      *back = 1;
      i--;
    }
    tokens[++i] = strtok(NULL, " ");
  }
  return tokens;
}

int internalcom(char** token) {
  int i = 0;
  for (i = 0; i < CMD_NUM; i++) {
    if (strcmp(token[0], cmd[i]) == 0) {
      return (*func[i])(token);
    }
  }
  return -1;
}
int ExecuteLine(char** token) {
  int fid = fork();
  if (fid < 0) {
    perror("fork error");
    return 1;
  } else if (fid == 0) {
    if (execvp(token[0], token) < 0) {
      exit(1);
    }
  } else if (fid > 0) {
    int status;
    wait(&status);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
      return 1;
    }
  }
  printf(" %s not found or stopped\n", token[0]);
  return 1;
}

int ExecuteSpecialLine(char** token) {
  char* special_tokens[] = {NULL};
  char* pre_commend[10] = {NULL};
  char* post_commend[10] = {NULL};
  char* execute_tokens[2] = {NULL};
  int output = 1;
  int sum = 0;
  int count = 0;
  int status = 1;
  int preorpost =
      0;  // 0: pre 1: post  /to judge the commends put into (pre/post)
  int TopPipe[2];
  pipe(TopPipe);
  char* pos = token[sum];
  // exxecute in the order of '<>|'
  while (pos != NULL) {
    if (strcmp(pos, "|") == 0) {
      preorpost = 1;
      if (count == 0) {
        execute_tokens[count++] = "|";
      } else {
        if (strcmp(execute_tokens[0], "|") == 0) {
          execute_tokens[0] = "|";
          status = my_pipe(pre_commend, post_commend, TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        } else if (strcmp(execute_tokens[0], ">") == 0) {
          execute_tokens[0] = "|";
          status = relocate(0, pre_commend, post_commend[0], TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        } else if (strcmp(execute_tokens[0], ">>") == 0) {
          execute_tokens[0] = "|";
          status = relocate(1, pre_commend, post_commend[0], TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        } else if (strcmp(execute_tokens[0], "<") == 0) {
          execute_tokens[0] = "|";
          status = relocate(2, pre_commend, post_commend[0], TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        }
      }
    } else if (strcmp(pos, ">") == 0) {
      output = 0;
      preorpost = 1;
      if (count == 0) {
        execute_tokens[count++] = ">";
      } else {
        if (strcmp(execute_tokens[0], "|") == 0) {
          execute_tokens[0] = ">";
          status = my_pipe(pre_commend, post_commend, TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        } else if (strcmp(execute_tokens[0], ">") == 0) {
          execute_tokens[0] = ">";
          status = relocate(0, pre_commend, post_commend[0], TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        } else if (strcmp(execute_tokens[0], ">>") == 0) {
          execute_tokens[0] = ">";
          status = relocate(1, pre_commend, post_commend[0], TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        } else if (strcmp(execute_tokens[0], "<") == 0) {
          execute_tokens[0] = ">";
          status = relocate(2, pre_commend, post_commend[0], TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        }
      }
    } else if (strcmp(pos, ">>") == 0) {
      output = 1;
      preorpost = 1;
      if (count == 0) {
        execute_tokens[count++] = ">>";
      } else {
        if (strcmp(execute_tokens[0], "|") == 0) {
          execute_tokens[0] = ">>";
          status = my_pipe(pre_commend, post_commend, TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        } else if (strcmp(execute_tokens[0], ">") == 0) {
          execute_tokens[0] = ">>";
          status = relocate(0, pre_commend, post_commend[0], TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        } else if (strcmp(execute_tokens[0], ">>") == 0) {
          execute_tokens[0] = ">>";
          status = relocate(1, pre_commend, post_commend[0], TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        } else if (strcmp(execute_tokens[0], "<") == 0) {
          execute_tokens[0] = ">>";
          status = relocate(2, pre_commend, post_commend[0], TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        }
      }
    } else if (strcmp(pos, "<") == 0) {
      preorpost = 1;
      if (count == 0) {
        execute_tokens[count++] = "<";
      } else {
        if (strcmp(execute_tokens[0], "|") == 0) {
          execute_tokens[0] = "<";
          status = my_pipe(pre_commend, post_commend, TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        } else if (strcmp(execute_tokens[0], ">") == 0) {
          execute_tokens[0] = "<";
          status = relocate(0, pre_commend, post_commend[0], TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        } else if (strcmp(execute_tokens[0], ">>") == 0) {
          execute_tokens[0] = "<";
          status = relocate(1, pre_commend, post_commend[0], TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        } else if (strcmp(execute_tokens[0], "<") == 0) {
          execute_tokens[0] = "<";
          status = relocate(2, pre_commend, post_commend[0], TopPipe);
          pre_commend[0] = NULL;
          post_commend[0] = NULL;
          if (status == 0) {
            return 1;
          }
        }
      }
    } else {
      if (preorpost == 0) {
        int i = 0;
        while (pre_commend[i] != NULL) {
          i++;
        }
        pre_commend[i] = pos;
        pre_commend[i + 1] = NULL;
      } else {
        int i = 0;
        while (post_commend[i] != NULL) {
          i++;
        }
        post_commend[i] = pos;
        post_commend[i + 1] = NULL;
      }
    }
    pos = token[++sum];
  }
  // deal with the last commend
  if (strcmp(execute_tokens[0], "|") == 0) {
    status = my_pipe(pre_commend, post_commend, TopPipe);
    pre_commend[0] = NULL;
    post_commend[0] = NULL;
    if (status == 0) {
      return 1;
    }
  } else if (strcmp(execute_tokens[0], ">") == 0) {
    status = relocate(0, pre_commend, post_commend[0], TopPipe);
    pre_commend[0] = NULL;
    post_commend[0] = NULL;
    if (status == 0) {
      return 1;
    }
  } else if (strcmp(execute_tokens[0], ">>") == 0) {
    status = relocate(1, pre_commend, post_commend[0], TopPipe);
    pre_commend[0] = NULL;
    post_commend[0] = NULL;
    if (status == 0) {
      return 1;
    }
  } else if (strcmp(execute_tokens[0], "<") == 0) {
    status = relocate(2, pre_commend, post_commend[0], TopPipe);
    pre_commend[0] = NULL;
    post_commend[0] = NULL;
    if (status == 0) {
      return 1;
    }
  }
  if (output) outputpipe(TopPipe);
  return 1;
}

void outputpipe(int TopPipe[]) {
  int fd = fork();
  if (fd < 0) {
    perror("fork error");
    exit(1);
  } else if (fd == 0) {
    close(TopPipe[1]);
    char buf[1024];
    int n;
    int flags = fcntl(TopPipe[0], F_GETFL);
    flags |= O_NONBLOCK;
    fcntl(TopPipe[0], F_SETFL, flags);
    while ((n = read(TopPipe[0], buf, 1024)) > 0) {
      if (n < 0) {
        perror("read error");
        exit(1);
      }
      if (n == 0) {
        break;
      }
      write(STDOUT_FILENO, buf, n);
    }
    exit(0);
  } else {
    close(TopPipe[1]);
    wait(NULL);
    close(TopPipe[0]);
  }
}

int relocate(int status, char** tokens, char* filename, int toppipe[]) {
  // status 0: >  1: >> 2: <
  int fid;            // fork id
  if (status == 0) {  // do >
    fid = fork();
    if (fid < 0) {
      perror("fork error");
      return 0;
    } else if (fid == 0) {
      int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      dup2(fd, STDOUT_FILENO);
      close(fd);
      if (tokens[0] == NULL) {  // no command,so must read input from pipe
        close(toppipe[1]);
        char buffer[1024];
        int flags = fcntl(toppipe[0], F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(toppipe[0], F_SETFL, flags);
        int len = 0;
        while ((len = read(toppipe[0], buffer, 1024)) > 0) {
          if (len == -1) {
            perror("read error");
            return 0;
          }
          if (len == 0) {
            break;
          }
          write(STDOUT_FILENO, buffer, len);
        }
        exit(0);
      }
      // have command,so execute command
      execvp(tokens[0], tokens);
      exit(1);
    } else if (fid > 0) {
      int status;
      close(toppipe[1]);
      wait(&status);
      if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 1;
      } else {
        printf("commend error\n");
        return 0;
      }
    }
  } else if (status == 1) {  // do >>
    fid = fork();
    if (fid < 0) {
      perror("fork error");
      return 0;
    } else if (fid == 0) {
      int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
      dup2(fd, STDOUT_FILENO);
      close(fd);
      if (tokens[0] == NULL) {  // no command,so must read input from pipe
        close(toppipe[1]);
        int flags = fcntl(toppipe[0], F_GETFL);
        flags |= O_NONBLOCK;
        fcntl(toppipe[0], F_SETFL, flags);
        int len = 0;
        char buffer[1024];
        while ((len = read(toppipe[0], buffer, 1024)) > 0) {
          if (len == -1) {
            perror("read error");
            return 0;
          }
          if (len == 0) {
            break;
          }
          write(STDOUT_FILENO, buffer, len);
        }
        exit(0);
      }
      // have command,so execute command
      execvp(tokens[0], tokens);
      exit(1);
    } else if (fid > 0) {
      int status;
      close(toppipe[1]);
      wait(&status);
      if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 1;
      } else {
        printf("commend error\n");
        return 0;
      }
    }
  } else if (status == 2) {  // do <
    fid = fork();
    if (fid < 0) {
      perror("fork error");
      return 0;
    } else if (fid == 0) {
      if (tokens[0] == NULL) {
        printf("commend error\n");
        exit(0);
      }
      int fd = open(filename, O_RDONLY);
      if (fd < 0) {
        // printf("commend error\n");
        exit(0);
      }
      dup2(fd, STDIN_FILENO);
      close(fd);
      close(toppipe[0]);
      dup2(toppipe[1], STDOUT_FILENO);
      close(toppipe[1]);
      execvp(tokens[0], tokens);
      exit(1);
    } else if (fid > 0) {
      int status;
      wait(&status);
      if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return 1;
      } else {
        printf("commend error\n");
        return 0;
      }
    }
  }
}

int my_pipe(char** pre_tokens, char** post_tokens, int TopPipe[]) {
  int stdfd = dup(0);
  int fd[2];
  pipe(fd);
  int fid = fork();
  if (fid < 0) {
    perror("fork error");
    return 1;
  } else if (fid == 0) {  // write into pipe
    close(fd[0]);
    dup2(fd[1], STDOUT_FILENO);
    close(fd[1]);
    if (pre_tokens[0] == NULL) {  // no command,so must read input from file
      close(TopPipe[1]);
      int flags = fcntl(TopPipe[0], F_GETFL);
      flags |= O_NONBLOCK;
      fcntl(TopPipe[0], F_SETFL, flags);
      char buffer[1024];
      int len = 0;
      while ((len = read(TopPipe[0], buffer, 1024)) > 0) {
        if (len == -1) {
          perror("read error");
          return 0;
        }
        if (len == 0) {
          break;
        }
        write(STDOUT_FILENO, buffer, len);
      }
      exit(0);
    }
    execvp(pre_tokens[0], pre_tokens);
    exit(1);
  } else if (fid > 0) {  // post
    int status;
    wait(&status);
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
      close(fd[1]);
      int fid2 = fork();
      if (fid2 < 0) {
        perror("fork error");
        return 0;
      } else if (fid2 == 0) {
        close(fd[1]);  // read from pipe
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        if (post_tokens[0] == NULL) exit(1);
        close(TopPipe[0]);
        dup2(TopPipe[1], STDOUT_FILENO);
        close(TopPipe[1]);
        execvp(post_tokens[0], post_tokens);
        exit(1);
      } else if (fid2 > 0) {
        int status2;
        wait(&status2);
        if (WIFEXITED(status2) && WEXITSTATUS(status2) == 0) {
          return 1;
        } else {
          printf("commend error\n");
          return 0;
        }
      }
    } else {
      printf("commend error\n");
      return 0;
    }
  }
}