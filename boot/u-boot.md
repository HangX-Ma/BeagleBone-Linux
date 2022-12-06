# U-Boot

U-Boot is the shortcut name of _'Universal Bootloader'_. The primary job of U-Boot is preparing the hardware to boot into Linux. U-Boot is responsible for initializing enough of the hardware so that the Linux kernel can be loaded into memory and begin its boot process.

U-Boot basically tells the BIOS (Basic Input Output System) to run zImage with the options that tell zImage where to find the root filesystem so it knows how to start.

## U-Boot Compilation

U-Boot source code can be found [here](https://source.denx.de/u-boot/u-boot.git) and the documentation on how to use and develop U-Boot can be found at <https://u‑boot.readthedocs.io>.

- **STEP 1:** Deletes all the previously compiled or generated objects files. We need to determine the cross compiler for it. Details is record in [corss-compiling.md](cross-compiling.md).

    ```bash
    make CROSS_COMPILE=arm-linux-gnueabihf- distclean
    ```

- **STEP 2:** Apply board default configuration for U-Boot. Because we actually use the _Das U-Boot Bootloader_, according to the suggestions on this [page](https://beagleboard.org/project/U-Boot+%28V1%29/), the configuration for BeagleBone is recommended to `configs/am335x_evm_defconfig`. But I find BeagleBone Black is not identical with am335x_evm board, `configs/am335x_boneblack_vboot_defconfig` seems to be more suitable, which uses device tree to describe all the peripherals. But I have tested it cannot perform normally. Only tag older than `v2019.01` contains `configs/am335x_boneblack_defconfig`, which can boot the board successfully.

    ```bash
    make CROSS_COMPILE=arm-linux-gnueabihf- am335x_boneblack_defconfig
    ```

- **STEP 3:** Run `menuconfig`, if you want to do any settings other than default configuration. I suggest to change the `Autoboot option` the `CONFIG_BOOTDELAY` from 2 to 10 for redundant time to enter U-Boot Command Terminal.

    ```bash
    make CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
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

By default, the U-Boot has set the address in memory to use at **0x8200000**, we don't need to change it. You can check this value in `menu/General Setup`.

### MLO plus u-boot.img Test Result

```bash
U-Boot 2019.01 (Dec 05 2022 - 19:14:46 +0800)

CPU  : AM335X-GP rev 2.1
I2C:   ready
DRAM:  512 MiB
No match for driver 'omap_hsmmc'
No match for driver 'omap_hsmmc'
Some drivers were not found
MMC:   OMAP SD/MMC: 0, OMAP SD/MMC: 1
Loading Environment from FAT... *** Warning - bad CRC, using default environment

Loading Environment from MMC... *** Warning - bad CRC, using default environment

<ethaddr> not set. Validating first E-fuse MAC
Net:   cpsw, usb_ether
Press SPACE to abort autoboot in 5 seconds
=>
```

## Dive into U-Boot

We already have `MLO` and `u-boot.img` to finish the RBL and SPL. To load the linux kernel using U-Boot, we need a uImage. We can use the uImage file to test our compilation result and decode the U-Boot header of uImage manually. Therefore, we need to compile the linux kernel to support our research.

### Linux Kernel Compilation

The BeagleBone official provide linux kernel on website <https://github.com/beagleboard/linux>.

#### The board file

The board file is a 'C' source file, which explains the various peripherals on the board (outside the SOC). This file usually contains the registration functions and data of various on board devices, eg, serial, gpio, i2c, mux, etc. Basically the board vendor uses board files to register various board peripherals with the linux subsystem.

The board or SOC specific codes go into `arch/arm`. You cannot find `am335x`, but `omap1`, `omap2` and `davinci`, which also provided by Texas Instruments. `davinci` is a dedicated SOC used on video, so its peripheral IPs are mighty different from `omap` family chip. But `am335x` is quite similar to `omap` family. We can use `omap` instead.

In `arch/arm/mach-omap2` directory, you can find a 'C' file named `board-generic.c`. When the linux detects a particular hardware, then it triggers the appropriate initialization functions. Single board file has support for multiple platforms, with the help the DTB file, you can make linux multi platform capable, no necessity to recompile the linux kernel. You can find a macro `CONFIG_SOC_AM33XX`. The linux detects the DTB, read the field called `.dt_compat` from the DTB, make comparison and then make appropriate initialization.

```c
#ifdef CONFIG_SOC_AM33XX
static const char *const am33xx_boards_compat[] __initconst = {
        "ti,am33xx",
        NULL,
};

DT_MACHINE_START(AM33XX_DT, "Generic AM33XX (Flattened Device Tree)")
        .reserve        = omap_reserve,
        .map_io         = am33xx_map_io,
        .init_early     = am33xx_init_early,
        .init_machine   = omap_generic_init,
        .init_late      = am33xx_init_late,
        .init_time      = omap_init_time_of,
        .dt_compat      = am33xx_boards_compat,
        .restart        = am33xx_restart,
MACHINE_END
#endif
```

#### About initarmfs and creating initarmfs

`initarmfs` actually abbreviates `initial`, `RAM based`, `file system`. This is nothing but a file system hierarchy made to live in the RAM of the device by compressing it and during booting, Linux mounts this file system as the initial file system. That means you just need RAM to mount the FS and get going with the complete boot process.

So, basically `initramfs` embedded into the kernel and loaded at an early stage of the boot process, where it gives all the minimal requirements to boot the Linux kernel successfully on the board just from RAM without worrying about other peripherals. And what you should store in initramfs is left to your product requirements, you may store all the important drivers and firmware, you may keep your product specific scripts, early graphic display logos, etc.

This [page](https://github.com/jeffegg/beaglebone/blob/master/Documentation/filesystems/ramfs-rootfs-initramfs.txt) discuss more details about what initarmfs actually is.

##### How to keep initramfs in to RAM?

1. You can make initramfs “built in” in to the Linux Kernel during compilation, so when the Linux starts booting , it will place the initramfs in the RAM and mounts as the initial root file system and continues. [Preferred]

2. You can load the initramfs from some other sources in to the RAM of your board and tell the Linux Kernel about it (At what RAM address initramfs is present) via the kernel boot arguments.

##### Generate independent initramfs

- Get into the target folder and run the below 2 commands. In the first command we are generating a cpio archive and then gz archive.

    ```bash
    find . | cpio -H newc -o > ../initramfs.cpio
    cat ../initramfs.cpio | gzip > ../initramfs.gz
    ```

- Install mkImage command. For that run “apt-get install u-boot-tools” on your terminal software, this will install all the u-boot related tools along with mkImage tool.

    ```bash
    sudo apt-get install u-boot-tools
    ```

- Make initramfs , U-Boot friendly by attaching the uboot header with load address and other info.

    ```bash
    mkimage -A arm -O Linux -T ramdisk -C none -a 0x80800000 -n "Root Filesystem" -d ../initramfs.gz ../initramfs
    ```

You will end up with a file “initramfs” which also includes the uboot header and this file we will be used as ram based file system whenever required.

#### Linux Kernel Compilation Steps

After you finish the _STEP3_, you can select to compile the drivers as modules or you can select drivers as a statically compiled kernel module. We usually don't touch upon the kernel related features and stuffs like that.

I need to explain the reason of setting `LOADADDR` to 0x80008000 in STEP 4. In [Booting ARM Linux](http://www.simtec.co.uk/products/SWLINUX/files/booting_article.html), _Loading the kernel image_, mentions that **Despite the ability to place zImage anywhere within memory, convention has it that it is loaded at the base of physical RAM plus an offset of 0x8000 (32K). This leaves space for the parameter block usually placed at offset 0x100, zero page exception vectors and page tables. This convention is very common.** Remember that `LOADADDR` is the kernel's start address (note that this is not the actual kernel run address), and uBoot will copy the kernel to this address (or not) and execute.

```bash
# STEP1: Clear previous compiled or generated objects files.
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- distclean
# STEP2: Select default configuration file bb.org_defconfig, for Kernel version newer than 4.11, using omap2plus_defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- bb.org_defconfig # 4.4
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- omap2plus_defconfig # 4.11
# STEP3: make menu
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
# STEP 4: We want to generate uImage and dtbs. Otherwise the kernel will default generate zImage
# We also mention the load address of the uImage.
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- uImage dtbs LOADADDR=0x80008000 -j8
# STEP 5: Generate modules you have selected
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j4 modules
# STEP 6: Modules installation (After you have create ROOTFS)
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=<path of the RFS> modules_install
```

Remember, the dynamically loadable kernel modules are not yet compiled (\<M> entries). You have to compile them separately. STEP 4 does this compilation and generates the linux kernel modules with extension of `.ko`. All those generated modules (`.ko` files) should be transferred to the Root File system. This step is called as 'modules installation', STEP 5.

I have tested the compilation, tag 4.4 runs well. However, tag version newer than 4.4 all stick into an error. I think this is related some unfinished work. Someone has requested this issue and hope it will be resolved soon.

##### $\downarrow$ Error occurs in tag version newer than 4.4

```bash
./include/linux/compiler.h:328:45: error: call to ‘__compiletime_assert_1140’ declared with attribute error: BUILD_BUG_ON failed: sizeof(_s) > sizeof(long)
```

If you can compile the kernel until STEP 3, information blow will be printed on the screen.

```bash
Image Name:   Linux-4.4.155
Created:      Tue Dec  6 16:20:42 2022
Image Type:   ARM Linux Kernel Image (uncompressed)
Data Size:    8916736 Bytes = 8707.75 KiB = 8.50 MiB
Load Address: 80008000
Entry Point:  80008000
  Image arch/arm/boot/uImage is ready
```

##### $\downarrow$ Error occurs in tag 4.4

When you compile modules, this error will display.

```bash
include/linux/module.h:130:14: error: ‘cleanup_module’ specifies less restrictive attribute than its target ‘tilcdc_drm_fini’: ‘cold’ [-Werror=missing-attributes]
    int init_module(void) __attribute__((alias(#initfn)));

include/linux/module.h:136:14: error: 'cleanup_module' specifies less restrictive attribute than its target 'mpc52xx_lpbfifo_driver_exit': 'cold' [-Werror=missing-attributes]
    void cleanup_module(void) __attribute__((alias(#exitfn)));
```

This [page](https://lkml.org/lkml/2019/6/6/507) notices me that modified them to directly using `__attribute__((__copy__(initfn)))` and `__attribute__((__copy__(exitfn))`, fixing the bug. Therefore, I draw lessons from the official kernel tag 5.4. ([L126-L138](https://github.com/beagleboard/linux/blob/5.4/include/linux/module.h#L126-L137))

```c
/* Each module must use one module_init(). */
#define module_init(initfn)             \
    static inline initcall_t __maybe_unused __inittest(void)        \
    { return initfn; }                  \
    int init_module(void) __attribute__((alias(#initfn),__copy__(initfn)));

/* This is only required if you want to be unloadable. */
#define module_exit(exitfn)             \
    static inline exitcall_t __maybe_unused __exittest(void)        \
    { return exitfn; }                  \
    void cleanup_module(void) __attribute__((alias(#exitfn),__copy__(exitfn)));
```

### Decode U-Boot header of uImage manually

We copy the `arch/arm/boot/uImage` and `arch/arm/boot/dts/am335x-boneblack.dtb` into the mircoSD card, partition `/BOOT`, together with previously loaded `MLO` and `u-boot.img`. Press S2 button to enter SD card boot mode and then press space to abort autoboot, entering U-Boot terminal.

1. First we load the uImage into DDR memory address at 0x82000000, which locates in MMC0(microSD) partition 1(/BOOT). `ls mmc 0:1 /` can check the files. You can type `help` to get essential messages about all the commands in U-Boot.

    ```bash
    load mmc 0:1 0x82000000 uImage
    ```

2. Use `md` command to check the contents at specific address. Remember, uImage contains a 64 bytes u-boot header. You can check this header to verify the loading process or the file integral.

    ```bash
    md.l 0x82000000 4 # read four words
    ```
