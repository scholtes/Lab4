/* Author: Garrett Scholtes
 * Date:   2015-09-28
 *
 * Lab4 - CS4029
 * 
 * This module uses ACPI to discover battery status,
 * and writes that status to a /proc file
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");

int init_module( void ) {
    printk("Lab4 battery module installing\n");

    return 0;
}

void cleanup_module( void ) {
    printk("Lab4 battery module uninstalling\n");

    return;
}