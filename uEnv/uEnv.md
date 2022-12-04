# How _uEnv.txt_ works?

[uEnv.txt](uEnv.txt) is an example of microSD card uEnv.txt configuration. On BeagleBone Black board, eMMC use the MMC1 interface and microSD card use the MMC0 interface. Remember we divide two partitions for microSD card? Let's take a look at some command applied on microSD u-boot.

**NOTE:** If you want to use uboot terminal, please press `space key` or `enter key` during the start of the board.

## microSD mmc U-Boot

The microSD card has been divided into two partitions.

1. Load the linux kernel image to the DDR memory from microSD card.

    ```bash
    load mmc 0:1 0x82000000 /BOOT/uImage
    ```

   - This command means we copy the `/boot/uImage` file from MMC0, partition1 to DDR memory at 0x82000000. uImage contains the linux kernel and the header of u-boot. MLO will not copies the u-boot into the internal RAM because the internal RAM is only 128 KB, obviously not fitting for u-boot. And this is a reason why we use MLO to manage memory.

   - **[NOTE]:** If you change the boot source to eMMC, the command will be changed to this.

        ```bash
        load mmc 1:2 0x82000000 /boot/uImage
        ```

2. Load the dtb image to the DDR memory from microSD card.

    ```bash
    load mmc 0:1 0x88000000 /BOOT/am335x-boneblack.dtb
    ```

    - Similarly, we copy `/boot/am335x-boneblack.dtb` from MMC0, partition1 to DDR memory at 0x88000000.

    - **[NOTE]:** If you change the boot source to eMMC, the command will be changed to this.

        ```bash
        load mmc 1:2 0x88000000 /boot/am335x-boneblack.dtb
        ```

3. Send bootargs to the linux kernel from uboot.

    ```bash
    setenv bootargs console=ttyO0,115200n8 root=/dev/mmcblk0p2 rw rootfstype=ext4 rootwait debug earlyprintk mem=512M
    ```

    - **console=ttyO0,115200n8:** The kernel has no idea, which serial port of the board is used for sending the boot logs. The BBB board uses UART0 as the serial debug terminal, which is enumerated as `/dev/ttyO0` by the serial driver. We also need to determine the baud rate and the data format.

    - **root=/dev/mmcblk0p2 rw rootfstype=ext4:** The linux has no idea from where exactly it should mount the file system. So we need to determine the location and the type. Because the eMMC is also divided into two parts, BOOT and ROOTFS, respectively enumerated to `/dev/mmcblk0p1` and `/dev/mmcblk0p2`.

    - **mem=512M:** We allocate 512M for memory usage.

    - **earlyprintk:** You can see the printout before the kernel serial device is initialized.

    - **debug:** Use debug mode.

    - **rootwait:** Wait indefinitely.

4. Boot from memory command

    ```bash
    bootm 0x82000000 - 0x88000000
    ```

5. Command to import env variables from memory address.

    ```bash
    env import -t <memory addr> <size in bytes>
    ```

I have to mention that `loadaddr` and `fdtaddr` are environment variables defined in uboot. We can use the default ones.
