# Cross Compiling

Under most Linux systems, the compilation toolchain uses the GNU libc (glibc) as the C standard library. This compilation toolchain is called the "host compilation toolchain". The machine on which it is running, and on which you’re working, is called the "host system".

As your embedded system has a different processor, you need a cross-compilation toolchain - a compilation toolchain that runs on your host system but generates code for your target system (and target processor). For example, if your host system uses x86 and your target system uses ARM, the regular compilation toolchain on your host runs on x86 and generates code for x86, while the cross-compilation toolchain runs on x86 and generates code for ARM.

The well-known cross-compilation toolchains such as `Linaro` for ARM, `Sourcery CodeBench` for ARM, x86_64 and etc, can be used to compile your target processor code. `crosstool-NG` provides another way to generate a customized toolchain by yourself.

## Download and install cross-compilation toolchain

- Select the latest GNU toolchain for ARM on Linaro [GNU Toolchain Integration Builds](https://snapshots.linaro.org/gnu-toolchain/), extracting it to your `/home/user`. Pay attention to the version of the archive. `uname -a` command shows the system architecture of your host machine.

- Export the path of the cross compilation toolchain so you can use it easily.

    ```bash
    sudo vim /home/user/.bashrc
    # Add the following code at the bottom of the file
    export $PATH=<where the Linaro toolchain locates>:$PATH
    ```
