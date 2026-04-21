# OS-Jackfruit — Supervised Multi-Container Runtime

## 1. Team Information

| Name   | SRN           |
| ------ | ------------- |
| Vainik | PES1UG24CS513 |
| Vibhav | PES1UG24CS527 |

---

## 2. Project Overview

This project implements a **lightweight container runtime** similar to Docker using **Linux system calls and kernel features**.

Key features:

* Process isolation using **namespaces**
* Filesystem isolation using **chroot**
* Container lifecycle management via a **supervisor**
* Logging using **pipes**
* CLI communication using **UNIX domain sockets**
* Resource monitoring using a **kernel module**

---

## 3. Prerequisites

* Ubuntu 22.04 / 24.04 (VM recommended)
* Secure Boot OFF
* Install dependencies:

```bash
sudo apt update
sudo apt install -y build-essential linux-headers-$(uname -r)
```

---

## 4. Setup Root Filesystem (Alpine)

```bash
mkdir rootfs
cd rootfs

wget https://dl-cdn.alpinelinux.org/alpine/v3.20/releases/x86_64/alpine-minirootfs-3.20.3-x86_64.tar.gz

tar -xzf alpine-minirootfs-3.20.3-x86_64.tar.gz
cd ..
```

---

## 5. Build Project

```bash
cd ~/proj/boilerplate
make
```

This builds:

* engine (runtime)
* monitor.ko (kernel module)
* memory_hog, cpu_hog, io_pulse (test programs)

---

## 6. Load Kernel Module

```bash
sudo insmod monitor.ko
sudo sysctl kernel.dmesg_restrict=0

ls -la /dev/container_monitor
```

✔ If device appears → kernel module working

---

## 7. Start Supervisor (Terminal 1)

```bash
cd ~/proj/boilerplate
sudo rm -f /tmp/mini_runtime.sock
sudo ./engine supervisor ../rootfs
```

Output:

```
Supervisor running...
```

---

## 8. Run Containers (Terminal 2)

### Start container

```bash
sudo ./engine start alpha ../rootfs /bin/sh
```

### Run command inside container

```bash
sudo ./engine start test ../rootfs /bin/hostname
```

---

## 9. List Running Containers

```bash
sudo ./engine ps
```

Example:

```
NAME    PID
alpha   12345
beta    12350
```

---

## 10. View Logs

```bash
cat logs/alpha.log
```

---

## 11. Show Container Isolation (IMPORTANT DEMO)

```bash
sudo ./engine start demo ../rootfs /bin/pwd
sleep 1
cat logs/demo.log
```

Output:

```
/
```

✔ proves container root is isolated

---

## 12. Run Multiple Containers

```bash
sudo ./engine start alpha ../rootfs /bin/hostname
sudo ./engine start beta ../rootfs /bin/hostname
sudo ./engine ps
```

---

## 13. Memory Monitoring Test

```bash
sudo cp memory_hog ../rootfs/

sudo ./engine start memtest ../rootfs /memory_hog --soft-mib 5 --hard-mib 10

dmesg | grep container_monitor
```

✔ Shows memory warnings / kills

---

## 14. CPU Scheduling Experiment

```bash
sudo cp cpu_hog ../rootfs/
sudo cp io_pulse ../rootfs/

# High priority vs Low priority
sudo ./engine start cpu_hi ../rootfs /cpu_hog --nice 0
sudo ./engine start cpu_lo ../rootfs /cpu_hog --nice 19

sleep 10

cat logs/cpu_hi.log
cat logs/cpu_lo.log
```

---

## 15. Clean Shutdown

```bash
sudo pkill engine
sudo rm -f /tmp/mini_runtime.sock
sudo rmmod monitor
```

---

## 16. Key Concepts Used

### 1. Namespaces

* CLONE_NEWPID → process isolation
* CLONE_NEWUTS → hostname isolation
* CLONE_NEWNS → mount isolation

### 2. chroot

* Changes root filesystem
* Container sees only its own files

### 3. Supervisor

* Manages all containers
* Prevents zombie processes
* Handles signals

### 4. IPC Mechanisms

* Pipes → logging
* UNIX domain sockets → command communication

### 5. Kernel Module

* Tracks memory usage (RSS)
* Enforces soft + hard limits

---

## 17. Important Commands (Viva Ready)

| Command           | Purpose         |
| ----------------- | --------------- |
| engine supervisor | start runtime   |
| engine start      | start container |
| engine ps         | list containers |
| cat logs          | see output      |
| sleep 1           | wait for logs   |
| dmesg             | kernel logs     |

---

## 18. Conclusion

This project demonstrates how:

* Containers are created using Linux primitives
* Isolation is achieved without Docker
* Kernel and user space work together
* Scheduling and memory control affect container behavior

---

## 19. Future Improvements

* Add network namespace
* Add cgroups for better resource control
* Interactive container shell (exec)
* Web UI for monitoring

---
