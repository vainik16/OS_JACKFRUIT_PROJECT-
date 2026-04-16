
# Mini Container Runtime

## Description
This project implements a container runtime using clone() and chroot().

## Features
- Process isolation using clone()
- Filesystem isolation using chroot()
- Multiple containers (alpha, beta)

## How to Run

Compile:
gcc engine.c -o engine

Run:
sudo ./engine

## Containers
- rootfs-alpha → container 1
- rootfs-beta → container 2

## Authors
- Vainik
- Vibhav
