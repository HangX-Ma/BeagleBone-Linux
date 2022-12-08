# U-Boot
<!-- TOC -->

- [U-Boot](#u-boot)
  - [1. U-Boot Compilation](#1-u-boot-compilation)
    - [1.1 Compilation Result](#11-compilation-result)
    - [1.2 MLO plus u-boot.img Test Result](#12-mlo-plus-u-bootimg-test-result)
  - [2. Dive into U-Boot](#2-dive-into-u-boot)
    - [2.1 Linux Kernel Compilation](#21-linux-kernel-compilation)
      - [The board file](#the-board-file)
      - [About initarmfs and creating initarmfs](#about-initarmfs-and-creating-initarmfs)
        - [How to keep initramfs in to RAM?](#how-to-keep-initramfs-in-to-ram)
        - [Generate independent initramfs](#generate-independent-initramfs)
      - [Linux Kernel Compilation Steps](#linux-kernel-compilation-steps)
        - [Error occurs in tag version newer than 4.4](#error-occurs-in-tag-version-newer-than-44)
        - [Error occurs in tag 4.4](#error-occurs-in-tag-44)
    - [2.2 Decode U-Boot header of uImage manually](#22-decode-u-boot-header-of-uimage-manually)
  - [3. Minimalist root file system](#3-minimalist-root-file-system)
    - [3.1 Busybox Compilation](#31-busybox-compilation)
    - [3.2 Install root file system modules](#32-install-root-file-system-modules)
    - [3.3 Further Operation](#33-further-operation)

<!-- /TOC -->

U-Boot is the shortcut name of _'Universal Bootloader'_. The primary job of U-Boot is preparing the hardware to boot into Linux. U-Boot is responsible for initializing enough of the hardware so that the Linux kernel can be loaded into memory and begin its boot process.

U-Boot basically tells the BIOS (Basic Input Output System) to run zImage with the options that tell zImage where to find the root filesystem so it knows how to start.

## 1. U-Boot Compilation

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

### 1.1 Compilation Result

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

### 1.2 MLO plus u-boot.img Test Result

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

## 2. Dive into U-Boot

We already have `MLO` and `u-boot.img` to finish the RBL and SPL. To load the linux kernel using U-Boot, we need a uImage. We can use the uImage file to test our compilation result and decode the U-Boot header of uImage manually. Therefore, we need to compile the linux kernel to support our research.

### 2.1 Linux Kernel Compilation

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

##### Error occurs in tag version newer than 4.4

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

##### Error occurs in tag 4.4

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

### 2.2 Decode U-Boot header of uImage manually

We copy the `arch/arm/boot/uImage` and `arch/arm/boot/dts/am335x-boneblack.dtb` into the mircoSD card, partition `/BOOT`, together with previously loaded `MLO` and `u-boot.img`. Press S2 button to enter SD card boot mode and then press space to abort autoboot, entering U-Boot terminal.

1. First we load the uImage into DDR memory address at 0x82000000, which locates in MMC0(microSD) partition 1(/BOOT). `ls mmc 0:1 /` can check the files. You can type `help` to get essential messages about all the commands in U-Boot.

    ```bash
    load mmc 0:1 0x82000000 uImage
    ```

2. Use `md` command to check the contents at specific address. Remember, uImage contains a 64 bytes u-boot header. You can check this header to verify the loading process or the file integral.

    ```bash
    md.l 0x82000000 4 # read four words
    ```

## 3. Minimalist root file system

**Busybox** is a software tool that enables you to create a customized root file system for your embedded linux. It is so flexible that you can remove all unwanted features, linux commands, directories, etc. This can meets your source requirements and fit a limited memory space requirement. The most important point is that busybox has the potential to significantly reduce the memory consumed by various linux commands by merging all the linux commands into one single binary.

### 3.1 Busybox Compilation

New version like 1.34.1 is quite different. For BeagleBone Black, you need to change 2 settings after you compile STEP 2.

1. We want to build all the linux commands source code as a static binary instead of dynamic.
   Setting -> Build Options -> [*] Build static binary (no shared libs) # Otherwise you need to load the modules.
2. Setting -> Build Options -> (arm-linux-gnueabihf-) Cross compiler prefix

If you don't determine the install prefix, the busybox will select `./install` folder by default.

```bash
# STEP 0: Please download the latest stable version busybox 
# STEP1: Clear previous compiled or generated objects files.
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- distclean
# STEP 2 : Apply default configuration and settings
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- defconfig
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- menuconfig
# STEP 3 : Compile
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -j8
# STEP 4 : generate the busy box binary and minimal file system 
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- CONFIG_PREFIX=<install_path> install
```

### 3.2 Install root file system modules

You must remember what we haven't finished build modules in [linux kernel compilation](#linux-kernel-compilation-steps)(STEP 6). Now we can do this step, and the RFS path needs to be set where we install the root file system generated by busybox.

```bash
cd <linux kernel>
make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- INSTALL_MOD_PATH=<path of the RFS> modules_install
```

If you check the `install` folder in busybox workspace, in `install/lib/modules/<your kernel version>`, you can find some files, whose prefixes are modules with different suffixes.

- The `modules.builtin` describe which modules statically installed in kernel.
- The `modules.dep` illustrate the dependencies relationships between the loadable kernel modules.
- The `modules.alias` contains alias extracted from the modules.

There exit two way to add/remove the loadable modules into the linux kernel, `modprobe` and `insmod`. `modprobe` is a smarter command, which can intelligently add or remove a module, relying on the information in `modules.dep`. It will load the dependencies first. Oppositely, the `insmod` only does the load action.

### 3.3 Further Operation

After all above done, error occurs because of the `fs` incompleteness. NFS assists us to solving errors conveniently, for which we only need to change files located at our host machine.

1. Lack of `/dev/ttyx`.

    ```txt
    can't open /dev/tty4: No such file or directory
    can't open /dev/tty3: No such file or directory
    can't open /dev/tty2: No such file or directory
    ```

   - `mkdir dev` under your busybox `install` folder or where your file system locates at.

2. can't run `'/etc/init.d/rcS'`: No such file or directory.

    Remember the end of control flow during linux boot is `init`. who finally launches the `init` userspace application. In busybox generated file system, you can find the `init` in `sbin` directory. There are two types of init programs in linux, `Busybox "init"` and `System V "init"`. The former tries to look for `rcS` script in `/etc/init.d/` and the latter looks for `inittab` script.

    Generally, we use `rcS` to launch services/daemons during setup. That means, if you have any scripts or services to execute during starting up of the linux kernel, you can execute all those scripts and commands in this `rcS` file.

    The S in `rcS` stands for start. Oppositely, K in `rcK` stands for kill. `rcS` will launch all the scripts which start with the letter S and the number along with S is the sequence, such as `S01logging`.

    ```bash
    cd <shared nfs folder>
    sudo mkdir -p etc/init.d
    cp <all the scripts in etc/init.d>
    ```

    I use the scripts provided by [niekiran/EmbeddedLinuxBBB](https://github.com/niekiran/EmbeddedLinuxBBB) in `scripts/etc/init.d` folder.

3. Starting network: ifup: can't open `'/etc/network/interfaces'`: No such file or directory.

    ```bash
    mkdir -p /etc/network
    touch interfaces
    vi interfaces
    ```

    - Add the following code

    ```bash
    # interfaces(5) file used by ifup(8) and ifdown(8)

    auto lo
    iface lo inet loopback

    auto eth0
    iface eth0 inet static
        address 192.168.7.2
        netmask 255.255.255.0
        network 192.168.7.0
        gateway 192.168.7.1

    auto usb0
    iface usb0 inet static
        address 192.168.6.2
        netmask 255.255.255.0
        network 192.168.6.0
        gateway 192.168.6.1
    ```

4. Starting network: run-parts: `/etc/network/if-pre-up.d`: No such file or directory
5. ifup: can't open `'/var/run/ifstate.new'`: No such file or directory

    - Just create those files.
    - Reuslt $\downarrow$.

    ```shell
    /etc/network # ifconfig
    ifconfig: /proc/net/dev: No such file or directory
    eth0      Link encap:Ethernet  HWaddr F4:B8:98:A9:36:2D  
            inet addr:192.168.7.2  Bcast:192.168.7.255  Mask:255.255.255.0
            UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
            Interrupt:173 

    lo        Link encap:Local Loopback  
            inet addr:127.0.0.1  Mask:255.0.0.0
            UP LOOPBACK RUNNING  MTU:65536  Metric:1

    usb0      Link encap:Ethernet  HWaddr EA:CF:A1:A8:FB:F4  
            inet addr:192.168.6.2  Bcast:0.0.0.0  Mask:255.255.255.0
            UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1

    /etc/network # ping 192.168.6.1
    PING 192.168.6.1 (192.168.6.1): 56 data bytes
    64 bytes from 192.168.6.1: seq=0 ttl=64 time=0.457 ms
    64 bytes from 192.168.6.1: seq=1 ttl=64 time=0.846 ms
    ^C
    --- 192.168.6.1 ping statistics ---
    2 packets transmitted, 2 packets received, 0% packet loss
    round-trip min/avg/max = 0.457/0.651/0.846 ms
    ```

...
