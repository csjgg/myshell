#include "prompt.h"
#include <fcntl.h>
#include <linux/limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include <readline/history.h>
#include <readline/readline.h>

#define MAX_CMD_LENGTH 15
#define MAX_JOB_NUM 10

typedef struct job {
  char *commends[MAX_CMD_LENGTH + 1];
  int filed[3];
  int valid;
} job;

/*
 * builtin commends
 */
char *cmd[] = {"cd", "exit"};
int Exit(void) { return -1; }

int cd(char **token) {
  if (token[1] == NULL) {
    chdir("/home");
  } else {
    if (chdir(token[1]) != 0) {
      perror("myshell: cd wrong");
      return 0;
    }
  }
  return 1;
}

/*
 * add my executealbe files' directory into the environment varible PATH
 * return the old PATH
 */
char *SetTheEnv(void) {
  char *path_tmp = (char *)malloc(PATH_MAX);
  getcwd(path_tmp, PATH_MAX);
  char *new_path = strcat(path_tmp, "/env");
  char *old_path = getenv("PATH");

  if (old_path == NULL) {
    setenv("PATH", new_path, 1);
  } else {
    char *path = (char *)malloc(strlen(old_path) + strlen(new_path) + 2);
    sprintf(path, "%s:%s", new_path, old_path);
    setenv("PATH", path, 1);
    free(path);
  }
  free(path_tmp);
  return old_path;
}

/*
 * init the joblist, make sure jobs' file descripters is right
 */
void initjoblist(job *joblist) {
  int i;
  for (i = 0; i < MAX_JOB_NUM; i++) {
    joblist[i].filed[0] = 0;
    joblist[i].filed[1] = 1;
    joblist[i].filed[2] = 2;
    joblist[i].valid = 0;
  }
}

/*
 * split commends into jobs and deal with the file descripters
 */
job *SplitLine(char *line, int *background, int *status) {
  char *token;
  *status = 1;
  int job_num = 0;
  int cmd_num = 0;
  job *joblist = (job *)malloc(sizeof(job) * MAX_JOB_NUM);
  initjoblist(joblist);
  token = strtok(line, " ");
  while (token != NULL) {
    if (job_num > MAX_JOB_NUM) {
      *status = 0;
      printf("myshell: Only can handle max 15 jobs\n");
      return joblist;
    }
    if (strcmp(token, "|") == 0) {
      joblist[job_num].commends[cmd_num] = NULL;
      joblist[job_num].valid = 1;
      int mpipe[2];
      pipe(mpipe);
      joblist[job_num].filed[1] = mpipe[1];
      joblist[job_num + 1].filed[0] = mpipe[0];
      cmd_num = 0;
      job_num++;
    } else if (strcmp(token, ">") == 0) {
      joblist[job_num].commends[cmd_num] = NULL;
      joblist[job_num].valid = 1;
      char *filename = strtok(NULL, " ");
      if (filename == NULL) {
        printf("myshell: no filename\n");
        *status = 0;
        return joblist;
      }
      int fid = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
      if (fid == -1) {
        *status = 0;
        printf("myshell: can not open file\n");
        return joblist;
      }
      joblist[job_num].filed[1] = fid;
      cmd_num = 0;
      job_num++;
    } else if (strcmp(token, ">>") == 0) {
      joblist[job_num].commends[cmd_num] = NULL;
      joblist[job_num].valid = 1;
      char *filename = strtok(NULL, " ");
      if (filename == NULL) {
        printf("myshell: no filename\n");
        *status = 0;
        return joblist;
      }
      int fid = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0666);
      if (fid == -1) {
        *status = 0;
        printf("myshell: can not open file\n");
        return joblist;
      }
      joblist[job_num].filed[1] = fid;
      cmd_num = 0;
      job_num++;
    } else if (strcmp(token, "<") == 0) {
      joblist[job_num].commends[cmd_num] = NULL;
      joblist[job_num].valid = 1;
      char *filename = strtok(NULL, " ");
      if (filename == NULL) {
        printf("myshell: no filename\n");
        *status = 0;
        return joblist;
      }
      int fid = open(filename, O_RDONLY);
      if (fid == -1) {
        *status = 0;
        printf("myshell: can not open file\n");
        return joblist;
      }
      joblist[job_num].filed[0] = fid;
      cmd_num = 0;
      job_num++;
    } else if (strcmp(token, "&") == 0) {
      joblist[job_num].commends[cmd_num] = NULL;
      joblist[job_num].valid = 1;
      *background = 1;
      job_num++;
      cmd_num = 0;
    } else if (strcmp(token, "&&") == 0) {
      joblist[job_num].commends[cmd_num] = NULL;
      joblist[job_num].valid = 1;
      cmd_num = 0;
      job_num++;
    } else {
      joblist[job_num].commends[cmd_num] = token;
      cmd_num++;
    }
    token = strtok(NULL, " ");
  }
  if (cmd_num != 0) {
    joblist[job_num].commends[cmd_num] = NULL;
    joblist[job_num].valid = 1;
  }
  return joblist;
}

/*
 * check the job list, and close all opened file, to make sure i/o is correct in
 * child process
 */
void Closefiles(job *joblist) {
  int i;
  for (i = 0; i < MAX_JOB_NUM; i++) {
    if (joblist[i].filed[0] != 0) {
      close(joblist[i].filed[0]);
    }
    if (joblist[i].filed[1] != 1) {
      close(joblist[i].filed[1]);
    }
    if (joblist[i].filed[2] != 2) {
      close(joblist[i].filed[2]);
    }
  }
}

/*
 * execute jobs in the order of job list
 */
int execute_cmd(job *joblist, int background) {
  int job_n = 0;
  int status;
  int fd;
  for (job_n = 0; joblist[job_n].valid == 1; job_n++) {
    if (strcmp(joblist[job_n].commends[0], "cd") == 0) {
      status = cd(joblist[job_n].commends);
      if (joblist[job_n].filed[1] != 1) {
        close(joblist[job_n].filed[1]);
      }
      if (joblist[job_n].filed[0] != 0) {
        close(joblist[job_n].filed[0]);
      }
      if (status == 0) {
        return 1;
      } else {
        continue;
      }
    } else if (strcmp(joblist[job_n].commends[0], "exit") == 0) {
      return Exit();
    } else {
      fd = fork();
      if (fd < 0) {
        perror("fork error");
        return 1;
      } else if (fd == 0) {
        if (background == 0) {
          signal(SIGINT, SIG_DFL);
        }
        if (joblist[job_n].filed[1] != 1) {
          dup2(joblist[job_n].filed[1], 1);
        }
        if (joblist[job_n].filed[0] != 0) {
          dup2(joblist[job_n].filed[0], 0);
        }
        execvp(joblist[job_n].commends[0], joblist[job_n].commends);
        // printf("worng\n");
        exit(22);
      } else if (fd > 0) {
        int status;
        if (joblist[job_n].filed[1] != 1) {
          close(joblist[job_n].filed[1]);
        }
        if (joblist[job_n].filed[0] != 0) {
          close(joblist[job_n].filed[0]);
        }
        waitpid(fd, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 22) {
          printf("myshell: execute failed / no such commend\n");
          return 1;
        }
        if (WIFSIGNALED(status)) {
          int signal_number = WTERMSIG(status);
          printf("Child process terminated by signal %d\n", signal_number);
          return 1;
        }
      }
    }
  }
  return 1;
}

int main() {
  char *line;
  job *joblist;
  int background = 0; // 0: foreground 1: background
  int status = 1;     // 0: exit 1: continue
  char *prompt;
  char *old_path = SetTheEnv();
  signal(SIGINT, SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  while (1) {
    // print the prompt and read commends
    prompt = genprompt();
    printf("%s", prompt);
    line = readline(NULL);
    free(prompt);
    if (line == NULL)
      break;
    add_history(line);
    // split commends, deal with file descripter
    joblist = SplitLine(line, &background, &status);
    if (joblist == NULL) {
      free(line);
      line = NULL;
      continue;
    }
    if (status == 0) {
      free(joblist);
      free(line);
      line = NULL;
      Closefiles(joblist);
      continue;
    }
    // execute commends
    if (background == 1) {
      int fd = fork();
      if (fd < 0) {
        perror("fork error");
      } else if (fd == 0) {
        status = execute_cmd(joblist, background);
        exit(status);
      } else {
        Closefiles(joblist);
        printf("(%d) %s\n", fd, joblist[0].commends[0]);
        background = 0;
      }
    } else {
      status = execute_cmd(joblist, background);
      if (status == -1) {
        free(joblist);
        free(line);
        break;
      }
    }
    free(joblist);
    free(line);
    line = NULL;
  }
  // reset the PATH
  if (old_path == NULL) {
    unsetenv("PATH");
  } else {
    setenv("PATH", old_path, 1);
  }
  return 0;
}