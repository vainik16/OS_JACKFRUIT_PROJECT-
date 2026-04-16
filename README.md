# Mini Container Runtime

This project implements a simple container runtime using Linux system calls such as clone() and chroot(). The objective is to understand how containers work internally by creating isolated processes and filesystems, similar to Docker.

Process isolation is achieved using clone(), which creates a new process with separate namespaces. Filesystem isolation is implemented using chroot(), which changes the root directory of the process to a new filesystem. The execv() system call is used to run a shell inside the container.

The project uses a base filesystem (rootfs-base) as a template. From this base, multiple container filesystems are created, namely rootfs-alpha and rootfs-beta. Each of these represents an independent container environment.

To run the project:

Compile the program:
gcc engine.c -o engine

Execute the container:
sudo ./engine

On successful execution, a shell prompt (bash-5.1#) appears, indicating that the container is running.

To switch between containers, modify the chroot path in the source code from "./rootfs-alpha" to "./rootfs-beta", then recompile and run the program again.

This project demonstrates key concepts such as process isolation, filesystem isolation, and containerization using low-level Linux system calls. It provides a basic understanding of how container runtimes like Docker are implemented internally.

## Authors
- Vainik (PES1UG24CS513)
- Vibhav (PES1UG24CS527)
