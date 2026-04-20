#define _GNU_SOURCE
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#define STACK_SIZE (1024 * 1024)
#define SOCKET_PATH "/tmp/mini_runtime.sock"

static char stack[STACK_SIZE];

typedef struct {
    char name[50];
    int pid;
} container_t;

container_t containers[100];
int container_count = 0;

/* ---------------- CONTAINER ---------------- */

int container_run(void *arg) {
    char **args = (char **)arg;

    setsid();

    if (chroot(args[1]) != 0) {
        perror("chroot failed");
        return 1;
    }

    chdir("/");
    execv(args[2], &args[2]);

    perror("exec failed");
    return 1;
}

/* ---------------- START WITH LOGGING ---------------- */

void start_container(char *name, char *rootfs, char *cmd) {

    int pipefd[2];
    pipe(pipefd);

    char logfile[100];
    sprintf(logfile, "logs/%s.log", name);

    int pid = fork();

    if (pid == 0) {
        close(pipefd[0]);

        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);

        char *args[] = {name, rootfs, cmd, NULL};

        clone(container_run, stack + STACK_SIZE,
              CLONE_NEWPID | CLONE_NEWUTS | CLONE_NEWNS | SIGCHLD,
              args);

        exit(0);
    } else {
        close(pipefd[1]);

        FILE *log = fopen(logfile, "w");

        char buffer[256];
        int n = read(pipefd[0], buffer, sizeof(buffer));

        if (n > 0) {
            fwrite(buffer, 1, n, log);
        }

        fclose(log);

        strcpy(containers[container_count].name, name);
        containers[container_count].pid = pid;
        container_count++;

        printf("Started container %s with PID %d\n", name, pid);

        // kernel hook
        int fd = open("/dev/container_monitor", O_RDWR);
        if (fd >= 0) {
            ioctl(fd, 1, pid);
            close(fd);
        }
    }
}

/* ---------------- SUPERVISOR ---------------- */

void supervisor() {
    int server_fd, client_fd;
    struct sockaddr_un addr;

    unlink(SOCKET_PATH);

    server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    bind(server_fd, (struct sockaddr *)&addr, sizeof(addr));
    listen(server_fd, 5);

    printf("Supervisor running...\n");

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);

        char buffer[256] = {0};
        read(client_fd, buffer, sizeof(buffer));

        char cmd[50] = {0}, name[50] = {0}, path[100] = {0}, exec_cmd[100] = {0};

        int count = sscanf(buffer, "%s %s %s %s", cmd, name, path, exec_cmd);

        if (count < 1) {
            close(client_fd);
            continue;
        }

        if (count == 4 && strcmp(cmd, "start") == 0) {
            start_container(name, path, exec_cmd);
        }
        else if (strcmp(cmd, "ps") == 0) {
            char out[512] = "NAME\tPID\n";
            for (int i = 0; i < container_count; i++) {
                char line[100];
                snprintf(line, sizeof(line), "%s\t%d\n",
                         containers[i].name, containers[i].pid);
                strcat(out, line);
            }
            write(client_fd, out, strlen(out));
        }
        else if (count >= 2 && strcmp(cmd, "stop") == 0) {
            for (int i = 0; i < container_count; i++) {
                if (strcmp(containers[i].name, name) == 0) {
                    kill(containers[i].pid, SIGKILL);
                }
            }
        }
        else {
            char *msg = "Unknown command\n";
            write(client_fd, msg, strlen(msg));
        }

        close(client_fd);
    }
}

/* ---------------- CLIENT ---------------- */

void send_command(char *cmd) {
    int sock;
    struct sockaddr_un addr;

    sock = socket(AF_UNIX, SOCK_STREAM, 0);

    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, SOCKET_PATH);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect (is supervisor running?)");
        return;
    }

    write(sock, cmd, strlen(cmd));

    char buffer[512] = {0};
    read(sock, buffer, sizeof(buffer));
    printf("%s", buffer);

    close(sock);
}

/* ---------------- MAIN ---------------- */

int main(int argc, char *argv[]) {

    container_count = 0;
    mkdir("logs", 0777);

    if (argc < 2) {
        printf("Usage:\n");
        printf("./engine supervisor <rootfs>\n");
        printf("./engine start <name> <rootfs> <cmd>\n");
        printf("./engine ps\n");
        printf("./engine stop <name>\n");
        return 1;
    }

    if (strcmp(argv[1], "supervisor") == 0) {
        supervisor();
    }
    else if (strcmp(argv[1], "start") == 0) {
        char cmd[256];
        sprintf(cmd, "start %s %s %s", argv[2], argv[3], argv[4]);
        send_command(cmd);
    }
    else if (strcmp(argv[1], "ps") == 0) {
        send_command("ps");
    }
    else if (strcmp(argv[1], "stop") == 0) {
        char cmd[100];
        sprintf(cmd, "stop %s", argv[2]);
        send_command(cmd);
    }
    else {
        printf("Unknown command\n");
    }

    return 0;
}
