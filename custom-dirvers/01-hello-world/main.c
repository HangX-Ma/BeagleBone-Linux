#include <linux/module.h>

/* Module initialization entry point */
static int __init hello_init(void) {
    pr_info("Hello World\n");
    return 0;
}

/* Module clean-up entry point */
static void __exit hello_cleanup(void) {
    pr_info("Goodbye World\n");
}

/* Registration of above entry points with kernel */
module_init(hello_init);
module_exit(hello_cleanup);

/* Descriptive information about module */
MODULE_LICENSE("Dual MIT/GPL");
MODULE_AUTHOR("HangX-Ma");
MODULE_DESCRIPTION("Hello world kernel module");
MODULE_INFO(board, "BeagleBone Black Rev C");
