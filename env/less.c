#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

#define width 100
#define height 100
typedef struct lines {
  char** line;
  int line_num;
  int line_used;
} lines;

int get_terminal_height();
void set_terminal_raw();  // set terminal raw mode
void reset_terminal();    // reset terminal
void insert_line(lines* file_lines, char* line);

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("please input file name\n");
    return 0;
  }
  char* filename = argv[1];
  FILE* fd = fopen(filename, "r");
  if (fd == NULL) {
    perror("file not exist\n");
    return 0;
  }
  // init the file_lines
  lines* file_lines = (lines*)malloc(sizeof(lines));
  file_lines->line_num = height;
  file_lines->line = (char**)malloc(sizeof(char*) * height);
  for (int i = 0; i < height; i++) {
    file_lines->line[i] = (char*)malloc(sizeof(char) * width);
  }
  file_lines->line_used = 0;

  int terminal_height = get_terminal_height() - 1;

  char commend;  // get the input commend
  int rows_printed = 0;

  // read the file and insert the line into file_lines
  while (1) {
    char read_line[width];
    char* judge = fgets(read_line, width, fd);
    if(judge != NULL)
    insert_line(file_lines, read_line);
    else break;
  }
  int row = 0;
  while (1) {
    printf("%s", file_lines->line[row++]);
    rows_printed++;
    if (rows_printed >= terminal_height - 1 || row >= file_lines->line_used) {
      printf("\n:");
      set_terminal_raw();
      commend = getchar();
      reset_terminal();
      printf("\033c");  // clear the screen

      // execute the commend
      if (commend == 'q')
        break;
      else if (commend == ' ') {
        if (row >= file_lines->line_used) {
          row = row - rows_printed;
          rows_printed = 0;
        } else {
          rows_printed = 0;
        }
      } else if (commend == '\n' || commend == 'k') {
        if (row >= file_lines->line_used) {
          row = row - rows_printed;
          rows_printed = 0;
        } else {
          row = row - rows_printed + 1;
          rows_printed = 0;
        }
      } else if (commend == 'b') {
        row = row - terminal_height - rows_printed;
        rows_printed = 0;
        if (row < 0) row = 0;
      } else if (commend == 'j') {
        row = row - rows_printed - 1;
        rows_printed = 0;
        if (row < 0) row = 0;
      } else if (commend == 'h') {
        printf("h: help\n");
        printf("q: quit\n");
        printf("b: back\n");
        printf("space: next page\n");
        printf("enter: next line\n");
        printf("j: next line\n");
        printf("k: previous line\n");
        printf(":");
        set_terminal_raw();
        getchar();
        reset_terminal();
        printf("\033c");
        row = row - rows_printed;
        rows_printed = 0;
      } else {
        printf("invalid commend\n");
        printf("\033c");
        row = row - rows_printed;
        rows_printed = 0;
      }
    }
  }
  fclose(fd);
  return 0;
}

int get_terminal_height() {
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  return w.ws_row;
}

void set_terminal_raw() {
  struct termios termios_p;
  tcgetattr(STDIN_FILENO, &termios_p);
  termios_p.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &termios_p);
}

void reset_terminal() {
  struct termios termios_p;
  tcgetattr(STDIN_FILENO, &termios_p);
  termios_p.c_lflag |= (ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &termios_p);
}

void insert_line(lines* file_lines, char* line) {
  if (file_lines->line_used == file_lines->line_num) {
    file_lines->line_num *= 2;
    file_lines->line =
        (char**)realloc(file_lines->line, sizeof(char*) * file_lines->line_num);
    for (int i = file_lines->line_used; i < file_lines->line_num; i++) {
      file_lines->line[i] = (char*)malloc(sizeof(char) * width);
    }
  }
  strcpy(file_lines->line[file_lines->line_used], line);
  file_lines->line_used++;
}