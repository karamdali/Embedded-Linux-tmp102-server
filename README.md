## Embedded Linux - TMP102 websocket server 

This project demonstrates a simple IoT solution using the BeagleBone Black (BBB) to monitor temperature in real-time via a web interface. A TMP102 sensor measures temperature, while a lightweight web server displays the readings on any connected device.

#### Key Features:

-   **Real-time temperature monitoring** via a web browser
    
-   **Minimal Linux setup** with custom kernel and BusyBox
    
-   **Network boot** (NFS) for easy development
    
-   **Lightweight WebSocket server** for live updates
    
-   **Low-power & embedded-friendly** design
    

Ideal for learning embedded Linux, device drivers, and IoT prototyping.

## Prerequisites

-   BeagleBone Black board
    
-   Host machine running Linux (Ubuntu/Debian recommended)
    
-   MicroSD card (4GB+) with FAT32 partition and BOOT flag is set.
    
-   TMP102 temperature sensor connected to BBB's I2C-2 bus
    
-   Network connection between BBB and host machine
    

## Setup Overview

1.  **Cross-Toolchain Setup**
    
2.  **U-Boot Compilation**
    
3.  **Linux Kernel with TMP102 Driver**
    
4.  **BusyBox Root Filesystem**
    
5.  **NFS Boot Configuration**
    
6.  **Device Tree Modification for I2C-2**
    
7.  **WebSocket Server Setup**


## **Building a Cross-Compiling Toolchain**

1.  **Install Dependencies**: Use `apt-get` to install tools like `build-essential`, `git`, and `autoconf`.
    
2.  **Download Crosstool-NG**: Clone the repository, checkout a stable version (e.g., `crosstool-ng-1.26.0`), and bootstrap it.
    
3.  **Configure & Build**: Run `./configure --enable-local`, then `make` to compile. Use `./ct-ng menuconfig` to set ARM Cortex-A8 target, Musl C library, and GCC 13.2.0.
    
4.  **Generate Toolchain**: Execute `./ct-ng build` to produce the toolchain in `~/x-tools/`.

## **Setting Up U-Boot for BBB**

1.  **Download U-Boot**: Clone the U-Boot repository and checkout a stable tag (e.g., `v2024.04`).
    
2.  **Configure & Compile**: Use `make am335x_evm_defconfig`, then compile with `make DEVICE_TREE=am335x-boneblack`.
    
3.  **Prepare SD Card**: Create a FAT32 partition with BOOT flag, copy `MLO` and `u-boot.img` to it.
4. **Configure Environment variables** 

## Linux Kernel Driver for TMP102 Temperature Sensor
**Kernel config option:** `CONFIG_TMP102=y` ,usually found under "Hardware Monitoring". 
Also, Ensure that your kernel supports booting the system using a root filesystem mounted over NFS.

## BusyBox Root Filesystem

In the BusyBox configuration interface, set the installation directory for BusyBox to the export path of NFS.

## NFS Boot Configuration
to boot from NFS from the host machine.  
`=> printenv bootargs`
`bootargs=console=ttyO0,115200n8 debuge earlyprintk root=/dev/nfs ip=192.168.0.100:::::eth0 nfsroot=192.168.0.101:/home/temp/fs,nfsvers=3,tcp rw`
`192.168.0.101` is the local IP address for the host, change it according to your setup.
`/home/temp/fs` is the NFS export path for the host.

## Device Tree Modification for I2C-2
We've used I2C-2 to connect the tmp102 to BBB and we need to let the kernel know about it so we 
### **Create a Device Tree Overlay**

    /dts-v1/; 
    /plugin/;
    
    / {
        fragment@0 {
            target = <&i2c2>;
            __overlay__ {
                #address-cells = <1>;
                #size-cells = <0>;
                tmp102@48 {
                    compatible = "ti,tmp102";
                    reg = <0x48>; //TMP i2c addr
                };
            };
        }; 
    };
### **Compile the Overlay**

    dtc -@ -O dtb -o BB-I2C2-TMP102.dtbo -b 0 BB-I2C2-TMP102.dts
### edit U-Boot environment variable bootargs 
to Load kernel, DTB, and overlay from MMC, merges them, and boots.
`=>printenv bootcmd`
`bootcmd=load mmc 0:1 ${loadaddr} zImage; load mmc 0:1 ${fdtaddr} am335x-boneblac                                                                                                             k.dtb; load mmc 0:1 ${overlayaddr} BB-I2C1-TMP102.dtbo; fdt addr ${fdtaddr}; fdt                                                                                                           resize 8192; fdt apply ${overlayaddr}; bootz ${loadaddr} - ${fdtaddr}`

## WebSocket Server Setup

Use your cross-compiling toolchain and compile server.c, ensure that mongoos.h and mongoos.c are in the same folder.

    arm-training-linux-musleabihf-gcc server.c mongoose.c -o server -I. -static

put the compiled server into your NFS and run it `./server`
P.s : change the `*s_http_addr = "http://192.168.0.100:8000";` with your BBB LAN IP

## Finally enter the BBB IP with the the server port into your browser (the pc should be on the same network LAN) 

### License

This code is provided AS IS without warranty. Feel free to use and modify it for any purpose.

### Author
Karam Dali - Damascus (21/04/2025)
