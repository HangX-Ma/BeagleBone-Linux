#include <linux/module.h>
#include <linux/platform_device.h>
#include "platform.h"


#undef pr_fmt
#define pr_fmt(fmt) "%s: " fmt,__func__

void pcdev_release(struct device *dev) {
    pr_info("Device released \n");
}


/* 1. Create 2 platform data */
struct pcdev_platform_data  pcdev_pdata[] = {
    [0] = {.size = 512, .perm = RDWR,   .serial_number = "PCDEVABC1111"},
    [1] = {.size = 1024,.perm = RDWR,   .serial_number = "PCDEVXYZ2222"},
    [2] = {.size = 128, .perm = RDONLY, .serial_number = "PCDEVXYZ3333"},
    [3] = {.size = 32,  .perm = WRONLY, .serial_number = "PCDEVXYZ4444"}
};

/* 2. Create 2 platform devices */

struct platform_device platform_device_1 = {
    
};