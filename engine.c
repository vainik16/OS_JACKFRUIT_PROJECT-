
#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 1024*1024

char stack[STACK_SIZE];

int run(void *arg) {
    chroot("./rootfs-alpha");
    chdir("/");

    char *args[] = {"/bin/bash", NULL};
    execv(args[0], args);

    perror("error");
    return 1;
}

int main(int argc, char *argv[]) {
    int pid = clone(run, stack + STACK_SIZE,
                    CLONE_NEWPID | CLONE_NEWUTS | SIGCHLD, NULL);

    waitpid(pid, NULL, 0);
    return 0;
}

