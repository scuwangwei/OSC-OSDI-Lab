# Overview
This repository contains lab implementations for the Operating System Capstone (Operating System Design and Implementation) on the Raspberry Pi 3B+ using bare-metal programming.

Firmware images in the `firmware` directory can be tested on real Raspberry Pi 3B+ hardware or via QEMU. For labs after Lab 2, the bootloader must be loaded first, and the kernel image is then transferred through UART.

# Hardware Platform
- Raspberry Pi 3B+
- UART for kernel image loading (after Lab 2

# Build
Build each lab using the provided Makefile.  
The Makefile also includes commands for running the system on QEMU.

# Labs
lab1 - Hello World  
lab2 - Booting Program  
lab3 - Exception and Interrupt  
lab4 - Allocator  
lab5 - Thread and System Call
