# Platform devices and drivers

## Discovery of devices

Every device has its configuration data and resources, which needs to be reported to OS. Enumeration is a process through which the OS can inquire and receive information, such as the type of the device, manufacturer, device configuration, etc. However, on embedded system platforms, most peripherals are connected to the CPU over buses that not support auto-discovery or enumeration of devices. But they must be part of the Linux device model, so the information about these devices must be fed to the Linux kernel manually either at compile time or at boot time of the kernel.

Generally, there exists three method to add platform devices information to Linux kernel.

- During compilation of kernel (deprecated)
  - Static method
  - Hardware details are part of kernel files (board file, drivers)
  - Details about the onboard peripheral and pin configuration, hard coded in kernel source files and board files
  - Recompile the kernel if any device property changes
- Loading dynamically
  - As a kernel module
  - Not recommended
- During kernel boot
  - Device tree blob
  - An operating system uses the Device Tree to discover the topology of the hardware at runtime, and thereby support a majority of available hardware without hard coded information
  - Latest and recommended

## Platform Bus

Platform bus is a pseudo bus or virtual bus, which doesn't have any physical existence. The bus interfaces have no auto discoverable and hot plugging capability.

## Platform Devices

A device that connected to the platform bus or if its parent bus doesn't support enumeration of connected devices will become a platform device.

### Driver Matching

The Linux platform core implementation maintains platform device and driver lists. Whenever you add a new platform device or driver, this list gets updated and matching mechanism triggers. The probe function will be called, inside which the driver configures the detected device. The details of the matched platform device will be passed to the probe function of the matched driver so that driver can extract the platform data and configure it.

Every bus type has its match function, where the device and driver list will be scanned.

#### Probe function

The probe function is responsible for

- Device detection and initialization
- Allocation of memories for various data structures
- Mapping I/O memory
- Registering interrupt handlers
- Registering device to kernel framework, user level access point creation, etc

#### Remove function

The remove function gets called when a platform device is removed from the kernel to unbind a device from the driver or when the kernel no longer use the platform device. It is responsible for

- Unregistering the device from the kernel framework
- Free memory if allocated on behalf of a device
- Shutdown/De-initialize the device

## Platform Drivers

A driver is in charge of handling a platform device.
