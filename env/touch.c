#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <utime.h>
#include <time.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("please input file name\n");
    return 1;
  }

  struct stat sb;
  if (stat(argv[1], &sb) == 0) {
    // update the file's modification time
    struct utimbuf new_times;
    new_times.actime = sb.st_atime;
    new_times.modtime = time(NULL);
    utime(argv[1], &new_times);
  } else {
    // create a new empty file
    FILE *fp = fopen(argv[1], "w");
    fclose(fp);
  }

  return 0;
}