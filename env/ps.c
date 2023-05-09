#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int isdigit_string(char *str) {
  int i = 0;
  while (str[i] != '\0') {
    if (!isdigit(str[i])) {
      return 0;
    }
    i++;
  }
  return 1;
}
int main() {
  DIR *dir = opendir("/proc");  // open the /proc directory
  if (!dir) {
    perror("opendir failed");
    return 1;
  }
  struct dirent *entry;
  int sigle = 0;
  while ((entry = readdir(dir)) != NULL) {
    if (isdigit_string(entry->d_name)) {  // is process
      if (sigle == 0) {
        printf("PID  ");
        printf("NAME\n");
        sigle = 1;
      }
      int pid = atoi(entry->d_name);
      char stat_path[256];
      sprintf(stat_path, "/proc/%d/stat", pid);
      FILE *fp_stat = fopen(stat_path, "r");  // open the stat file
      if (fp_stat) {
        char buf[2048];
        fgets(buf, sizeof(buf), fp_stat);
        fclose(fp_stat);
        char name[256];
        char *p1 = strchr(buf, '(');
        char *p2 = strrchr(buf, ')');
        if (p1 && p2) {
          size_t len = p2 - p1 - 1;
          memcpy(name, p1 + 1, len);
          name[len] = '\0';
        }
        if (pid > 500) {
          printf("%d  ", pid);
          printf("%s\n", name);
        }
      } else {
        perror("fopen failed");
      }
    }
  }

  closedir(dir);
  return 0;
}