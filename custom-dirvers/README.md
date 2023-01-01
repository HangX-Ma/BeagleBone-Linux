# README

## 06-pcd-sysfs

- Operation steps

```shell
# Create device with max_size and serial_number attributes
cd custom-drivers/05-pcd-platform && make host
sudo insmod pcd_device_setup.ko
# Create sysfs model which can read or write the device attributes
cd custom-drivers/06-pcd-sysfs && make host
sudo insmod pcd_sysfs.ko
```

- Result, you can find `max_size` and `serial_number` files in the following directory.

```shell
cd /sys/class/pcd_class/pcdev-0
```

## IntelliSense Setting for Kernel Programing

```json
{
    "configurations": [
        {
            "name": "Linux",
            "includePath": [
                "${workspaceFolder}/**",
                "<kernel directory prefix>/linux/include",
                "<kernel directory prefix>/linux/arch/arm/include",
                "<kernel directory prefix>/linux/arch/arm/include/generated"

            ],
            "defines": [
                "__GNUC__",
                "__KERNEL__",
                "MODULE"
            ],
            "compilerPath": "/usr/bin/gcc",
            "cStandard": "gnu17",
            "cppStandard": "gnu++14",
            "intelliSenseMode": "linux-gcc-x64"
        }
    ],
    "version": 4
}
```
