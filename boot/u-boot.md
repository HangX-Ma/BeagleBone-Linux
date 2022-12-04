# U-Boot

U-Boot is the shortcut name of _'Universal Bootloader'_. The primary job of U-Boot is preparing the hardware to boot into Linux. U-Boot is responsible for initializing enough of the hardware so that the Linux kernel can be loaded into memory and begin its boot process.

U-Boot basically tells the BIOS (Basic Input Output System) to run zImage with the options that tell zImage where to find the root filesystem so it knows how to start.

## U-Boot Compilation

U-Boot source code can be found [here](https://source.denx.de/u-boot/u-boot.git) and the documentation on how to use and develop U-Boot can be found at <https://u‑boot.readthedocs.io>.

- **STEP 1:** Deletes all the previously compiled or generated objects files. We need to determine the cross compiler for it. Details is record in [corss-compiling.md](cross-compiling.md).

    ```bash
    make CROSS_COMPILE=arm-linux-gnueabihf- distclean
    ```

- **STEP 2:** Apply board default configuration for u-boot. Because we actually use the _Das U-Boot Bootloader_, according to the suggestions on this [page](https://beagleboard.org/project/U-Boot+%28V1%29/), the configuration for BeagleBone is recommended to `configs/am335x_evm_defconfig`. But I find BeagleBone Black is not identical with am335x_evm board, `configs/am335x_boneblack_vboot_defconfig` is more suitable, which uses device tree to describe all the peripherals.

    ```bash
    make CROSS_COMPILE=arm-linux-gnueabihf- am335x_boneblack_vboot_defconfig
    ```

- **STEP 3:** Run `menuconfig`, if you want to do any settings other than default configuration. I suggest to change the `Autoboot option` the `CONFIG_BOOTDELAY` from 2 to 10 for redundant time to enter U-Boot Command Terminal.

    ```bash
    make CROSS_COMPILE=arm-linux-gnueabihf-  menuconfig
    ```

- **STEP 4:** Compile.

    ```bash
    make CROSS_COMPILE=arm-linux-gnueabihf- -j8
    ```

After all steps have been done, the `spl/u-boot-spl.bin` will be copied to generate `MLO` through `MKIMAGE MLO` command. Remember `MLO` and `u-boot.img` needs to be copied into `/BOOT/` partition in microSD card if you want to boot from microSD card.

### Compilation Result

```bash
api        configs   fs           Makefile      spl         u-boot.cfg.configs  u-boot-nodtb.bin
arch       disk      include      MLO           System.map  u-boot.dtb          u-boot.srec
board      doc       Kbuild       MLO.byteswap  test        u-boot-dtb.bin      u-boot.sym
boot       drivers   Kconfig      net           tools       u-boot-dtb.img
cmd        dts       lib          post          u-boot      u-boot.img
common     env       Licenses     README        u-boot.bin  u-boot.lds
config.mk  examples  MAINTAINERS  scripts       u-boot.cfg  u-boot.map
```
