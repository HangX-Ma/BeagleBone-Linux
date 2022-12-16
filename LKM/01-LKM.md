# Linux Kernel Module (LKM)

Linux support dynamic insertion and removal of code from the kernel while the system is up and running. The code what we add and remove at run time is called a kernel module. It can dynamically extend the functionality of the kernel by introducing new features to the kernel without even restarting the device. Hot-pluggable device, such as USB, is a good example.

We can divide LKMs to two types, static(y) and dynamic(m).

**Static(y):** The modules can statically linked to the kernel image when building the linux kernel. It is a 'built-in' method which increases the image size and occupies the memory permanently during runtime.

**Dynamic(m):** The modules are separately compiled and linked to produce `.ko` files. User space program, such as `insmod`, `modprobe`, `rmmod` can be used to dynamically load and upload these modules.

## Building a linux kernel module

Kernel module can be built in 2 ways: statically linked against the kernel image or dynamically loadable. The dynamically loadable kernel modules are also divided into 2 types, _In Tree Modules_ and _Out of Tree Modules_. Something needs to be mentioned that _Out of Tree Modules_ will taint the kernel, who will issue a warning when unknown modules loaded.

### Out of Tree Method Building

'kbuild' is used to build linux kernel to stay compatible with changes in the build infrastructure and pick up right flags to GCC. If we want to build an external modules, we need a prebuilt kernel source available contains configuration and header files used in the build. This can help the custom drivers keep track of the changes of the linux configuration.

Note that a module compiled against one linux kernel version can not be loaded into a system with another different kernel version. Precompiled linux kernel source tree is also needed. Install the Linux-headers-of the target linux kernel or build kernel by yourself can acquire the prebuilt linux kernel version.

#### Command Syntax

```bash

# Top level makefile will utilize the Kbuild rules, including 
# compiler switches, dependency list, version string, to build 
# your linux kernel.

make -C <path to linux kernel tree> M=<path to your module> [target]
```

- **-C**: The directory where the kernel source is located.
- **M**: Informs kbuild that an external module is being built. This requires an absolute path.
- **Target**: `modules`, `modules_install`, `clean`, `help`.
  - `modules`: The default target is `modules`.
  - `modules_install`: Install the external modules, whose default location is `lib/modules/<kernel_release>/extra/`. Prefix can be added with `INSTALL_MOD_PATH`.
  - `clean`: Remove all generated files in the modules directory only.
  - `help`: List available targets for external modules.

`file`, `modinfo` commands can be used to check the description of `.ko` files.

#### Local Makefile

```bash
obj-<X> := <module_name>.o
```

- X = n, Do not compile the module
- X = y, Compile the modules and link with kernel image
- X = m, Compile as dynamically loadable kernel module

### In Tree Method Building

In tree method building asks you to add the linux kernel modules inside the linux kernel source tree and let the linux build system builds that. Using `Kconfig` file can list the module selection in kernel menuconfig.

Suppose a situation that you need to add a module into linux character device type. You need to add your `m_dev` folder into the `drivers/char`. The simplest folder will contain the following files.

```txt
m_dev
├── main.c 
├── Makefile
└── Kconfig
```

`Kconfig` has a template below.

```txt
menu "custom modules"
config CUSTOM_MODULES_HW
        tristate "hello world module support"
        default n
endmenu
```

`Makefile` has a template below. The `$(CONFIG_CUSTOM_MODULES_HW)` will obtains the user defined module type.

```M
obj-$(CONFIG_CUSTOM_MODULES_HW) += main.o
```

Afterwards, in `drivers/char` directory, an upper level Kconfig can be found. Source your customized device Kconfig.

```txt
source "drivers/char/m_dev/Kconfig"
```

Another thing needs to do is add the local level Makefile to higher level Makefile in `dirvers/char` directory.

```M
obj-y               += m_dev
```

#### NOTE

If you want to use `sudo make` with compiler you have downloaded, like `arm-linux-gnueabihf-`, please add `export PATH=<your toolchain path:$PATH` at `Defaults secure_path`, in `/etc/sudoers`.
