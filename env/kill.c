#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    pid_t pid;

    if (argc < 2) {
        return 1;
    }
    pid = atoi(argv[1]); 
    if (kill(pid, SIGTERM) < 0) {
        perror("kill");
        exit(1);
    }
    return 0;
}
