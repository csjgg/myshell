#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

int IS_DIR(char *path);
int compare_names(const void *a, const void *b);  // sort the names

void outputfile_l(char *path, char *name);

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
  char path[1024];       // Path
  int beginnum = 0;
  if (status == 1) {
    if (argc == 2) {
      strcpy(path, ".");  // Open current directory
      beginnum = 1;
    } else {
      strcpy(path, argv[2]);
      beginnum = 2;
    }
  } else {
    if (argc == 1) {
      strcpy(path, ".");  // Open current directory
      beginnum = 0;
    } else {
      strcpy(path, argv[1]);
      beginnum = 1;
    }
  }
  for (i = beginnum; i < argc; i++) {  // open the special dir
    if (beginnum&&beginnum !=1) {
      strcpy(path, argv[i]);
    }
    if (IS_DIR(path)) {
      char *filenames[100];
      dir = opendir(path);
      if (dir == NULL) {
        perror("opendir");
        return 0;
      }
      int num_files = 0;
      int maxwidth = 0;
      while ((entry = readdir(dir)) != NULL) {
        filenames[num_files] = strdup(entry->d_name);
        strlen(filenames[num_files]) > maxwidth
            ? maxwidth = strlen(filenames[num_files])
            : maxwidth;
        num_files++;
      }
      qsort(filenames, num_files, sizeof(char *), compare_names);  // sort the name by char order
      int linesize = 10;
      int filenum = 0;
      maxwidth += 2;
      for (int j = 0; j < num_files; j++) {
        if (status == 0) {
          if (strcmp(filenames[j], ".") == 0 ||
              strcmp(filenames[j], "..") == 0) {
            continue;
          }
          if (filenames[j][0] == '.') {
            continue;
          }
          int len = strlen(filenames[j]);
          printf("%.*s%*s", len, filenames[j], maxwidth - len, "");
          if (j != num_files - 1) {
            if (filenum && filenum % linesize == 0) {
              printf("\n");
            } else {
            }
          } else {
            printf("\n");
          }
          filenum++;
        } else {
          outputfile_l(path, filenames[j]);
        }
      }
      closedir(dir);
    } else {
      if (status == 0) {
        printf("%s", path);
        printf("\n");
      } else {
        outputfile_l(".", path);
      }
    }
  }
}

int IS_DIR(char *path) {  // judge dir
  struct stat st;
  if (stat(path, &st) != 0) {
    return 0;
  }
  return S_ISDIR(st.st_mode);
}

int compare_names(const void *a, const void *b) {
  const char *name1 = *(const char **)a;
  const char *name2 = *(const char **)b;
  return strcmp(name1, name2);
}

void outputfile_l(char *path, char *name) {
  struct stat st;     // File status
  struct passwd *pw;  // User
  struct group *grp;  // Group
  char mtime[80];     // Time of last modification
  char entry_path[1024];
  sprintf(entry_path, "%s/%s", path, name);
  if (lstat(entry_path, &st) == -1) {
    perror("lstat failed");
    return;
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
  printf(" %6lld", (long long)st.st_size);

  // print time of last modification
  strftime(mtime, 80, "%b %2d %H:%M", localtime(&st.st_mtime));

  printf(" %s", mtime);
  if(S_ISDIR(st.st_mode)){
    printf(" \033[34m%s\033[0m", name);
  }else if((st.st_mode & S_IXUSR)||(st.st_mode & S_IXGRP)||(st.st_mode & S_IXOTH)){
    printf(" \033[32m%s\033[0m", name);}
  else{
    printf(" \033[33m%s\033[0m", name);
  }
  printf("\n");
}
