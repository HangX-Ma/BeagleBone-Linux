# How _uEnv.txt_ works?



**NOTE:** If you want to use uboot terminal, please press space key during the start of the board.

1. Load the linux kernel image to the DDR memory from eMMC.

    ```bash
    load mmc 1:2 0x82000000 /boot/uImage
    ```

2. Load the dtb image to the DDR memory from eMMC.

    ```bash
    load mmc 1:2 0x88000000 /boot/am335x-boneblack.dtb
    ```

3. Send bootargs to the linux kernel from uboot.

    ```bash
    setenv bootargs console=ttyO0,115200 root=/dev/mmcblk0p2 rw
    ```

4. Boot from memory command

    ```bash
    bootm 0x82000000 - 0x88000000
    ```

5. Command to import env variables from memory address.