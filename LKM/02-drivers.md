# Device Driver

Device driver is a piece of code that configures and manages a device. It exposes interfaces to the user-space so that the user application can communicate with the device. There are various categories of linux device drivers, such as character device drivers, block device drivers(sdmmc, eeprom, flash, etc), network device drivers(ethernet, wifi, bluetooth, etc).

## Character driver (char driver)

Character driver accesses data from the device sequentially byte by byte not in chunks, without any sophisticated buffering strategies. eg. RTC, keyboard, sensors, serial port, parallel port, etc.

## Block driver

The device which handles data in chunks or blocks is called a block device. This type of drivers need should implement advanced buffering strategies to read and write to the block devices, and disk caches are involved. eg. hard disk, SDMMC, Nand flash, USB camera, etc.

## Device files

The driver has to create a interface at the user-space, specially called as device file. A device file gets populated in `/dev` directory during kernel boot time or device/driver hot plug events. Generally, device files are managed as part of VFS subsystem of the kernel.
