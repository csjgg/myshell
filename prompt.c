#include "prompt.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <pwd.h>


// get the prompt
char* commendline(void) {
  char* line = (char*)malloc(1024);
  char* red_color = (char*)malloc(6);
  strcpy(red_color, "\033[31m");
  char* reset_color = (char*)malloc(5);
  strcpy(reset_color, "\033[0m");
  char* green_color = (char*)malloc(6);
  strcpy(green_color, "\033[32m");
  char* blue_color = (char*)malloc(6);
  strcpy(blue_color, "\033[34m");
  // get the path
  char* dir = (char*)malloc(PATH_MAX+11);
  char* cwd;
  cwd = getcwd(dir, PATH_MAX);
  if (cwd == NULL) {
    dir[0] = '\0';
  }
  char* home_dir = getenv("HOME");
  if (home_dir) {
    size_t home_dir_len = strlen(home_dir);
    size_t path_len = strlen(dir);
    if (path_len >= home_dir_len && strncmp(dir, home_dir, home_dir_len) == 0) {
      // 将家目录部分替换为 "~"
      memmove(dir, "~", 1);
      memmove(dir + 1, dir + home_dir_len, path_len - home_dir_len + 1);
    }
  }
  char* tmp = (char*)malloc(PATH_MAX+11);
  strcpy(tmp, dir);
  memmove(dir + 5, tmp, strlen(tmp) + 1);
  memmove(dir, green_color, 5);
  strcat(dir, reset_color);
  free(tmp);
  char* user = malloc(64);
  uid_t uid = getuid();
  struct passwd *pw = getpwuid(uid);
  if (pw == NULL) {
    perror("Cannot get passwd entry");
    user[0] = '\0';
  }else{
    strcpy(user, pw->pw_name);
    user[strlen(pw->pw_name)] = '@';
    user[strlen(pw->pw_name)+1] = '\0';
  }
  char hostname[PATH_MAX];
  if (gethostname(hostname, sizeof(hostname)) == -1) {
    perror("Cannot get hostname");
    hostname[0] = '\0';
  }

  // cat
  strcpy(line, blue_color);
  strcat(line, user);
  free(user);
  strcat(line, hostname);
  //free(hostname);
  strcat(line, reset_color);
  strcat(line, ":");
  strcat(line, dir);
  free(dir);
  strcat(line, reset_color);
  strcat(line, red_color);
  strcat(line, "$ ");
  strcat(line, reset_color);
  free(blue_color);
  free(red_color);
  free(green_color);
  free(reset_color);
  return line;
}