#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char **argv) {
  int status = 0;  // 0: normal 1: -l

  int i;
  for (i = 0; i < argc; i++) {
    if (strcmp(argv[i], "-l") == 0) {
      status = 1;
    }
  }

  DIR *dir;              // Directory
  struct dirent *entry;  // Directory entry
  struct stat st;        // File status
  struct passwd *pw;     // User
  struct group *grp;     // Group
  char mtime[80];        // Time of last modification
  char *path;            // Path
  if (argc - status == 1) {
    path = ".";  // Open current directory
  } else {
    path = argv[i == 1 ? 2 : 1];  // Open directory
  }
  dir = opendir(path);
  if (dir == NULL) {
    perror("opendir");
    return 1;
  }

  while ((entry = readdir(dir)) != NULL) {
    if (status == 1) {
      char entry_path[1024];
      sprintf(entry_path, "%s/%s", path, entry->d_name);
      if (lstat(entry_path, &st) == -1) {
        perror("lstat failed");
        continue;
      }
      printf((S_ISDIR(st.st_mode)) ? "d" : "-");
      printf((st.st_mode & S_IRUSR) ? "r" : "-");
      printf((st.st_mode & S_IWUSR) ? "w" : "-");
      printf((st.st_mode & S_IXUSR) ? "x" : "-");
      printf((st.st_mode & S_IRGRP) ? "r" : "-");
      printf((st.st_mode & S_IWGRP) ? "w" : "-");
      printf((st.st_mode & S_IXGRP) ? "x" : "-");
      printf((st.st_mode & S_IROTH) ? "r" : "-");
      printf((st.st_mode & S_IWOTH) ? "w" : "-");
      printf((st.st_mode & S_IXOTH) ? "x" : "-");

      printf(" %ld", (long)st.st_nlink);

      // print owner name and group name
      pw = getpwuid(st.st_uid);
      grp = getgrgid(st.st_gid);
      printf(" %s %s", pw->pw_name, grp->gr_name);

      // print size of file
      printf(" %lld", (long long)st.st_size);

      // print time of last modification
      strftime(mtime, 80, "%b %d %H:%M", localtime(&st.st_mtime));

      printf(" %s", mtime);
    }
    if(status == 0 && (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name,"..")==0)) continue;
    printf("  %s", entry->d_name);
    if (status == 1) printf("\n");
  }
  if(status == 0) printf("\n");
  closedir(dir);
  return 0;
}
