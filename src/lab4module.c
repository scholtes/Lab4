/* Author: Garrett Scholtes
 * Date:   2015-09-28
 *
 * Lab4 - CS4029
 * 
 * This module uses ACPI to discover battery status,
 * and writes that status to a /proc file
 *
 * My system's call (at /sys/class/power_supply/BAT0/device/path):
 *    \_SB_.PCI0.BAT0
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define procfs_name "lab4battery"

MODULE_LICENSE("GPL");

static struct proc_dir_entry *lab4_procfile;
static int soopercount = 0;

int lab4_proc_show(struct seq_file *m, void *v) {
    seq_printf(m, "soopercount = %d", soopercount);
    soopercount += 1;
    return 0;
}

int lab4_proc_open(struct inode *inode, struct file *file) {
    return single_open(file, lab4_proc_show, NULL);
}

static const struct file_operations proc_file_fops = {
    .owner = THIS_MODULE,
    .open = lab4_proc_open,
    .read = seq_read,
    .llseek = seq_lseek,
    .release = single_release
};

int init_module( void ) {
    printk("Lab4 battery module installing\n");

    // Create the /proc file

    lab4_procfile = proc_create(procfs_name, 0, NULL, &proc_file_fops);

    printk("/proc file created for Lab4 battery");

    return 0;
}

void cleanup_module( void ) {
    printk("Lab4 battery module uninstalling\n");

    remove_proc_entry(procfs_name, NULL);

    return;
}