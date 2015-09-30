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
#include <linux/acpi.h>
#include <asm/i387.h>

#define procfs_name "lab4battery"

MODULE_LICENSE("GPL");

static struct proc_dir_entry *lab4_procfile;

int *get_battery_status(void) {
    static int ret[5] = {1, 2, 3, 4, 5};

    acpi_status status;
    acpi_handle handle;
    union acpi_object *result;
    struct acpi_buffer buffer = {ACPI_ALLOCATE_BUFFER, NULL};
    int chrg_dischrg, charge, y;

    status = acpi_get_handle(NULL, (acpi_string)"\\_SB_.PCI0.BAT0", &handle);

    status = acpi_evaluate_object(handle, "_BST", NULL, &buffer);
    result = buffer.pointer;

    if(result) {
        chrg_dischrg = result->package.elements[0].integer.value;
        charge = result->package.elements[2].integer.value;
        kernel_fpu_begin();
        y = (int)(charge/1.0);
        kernel_fpu_end();
        ret[0] = chrg_dischrg;
        ret[1] = charge;
        ret[2] = y;
        kfree(result);
    }

    return ret;
}

int lab4_proc_show(struct seq_file *m, void *v) {
    int *bat_info;
    bat_info = get_battery_status();
    seq_printf(m, "%d %d %d %d %d\n",
        bat_info[0],
        bat_info[1],
        bat_info[2],
        bat_info[3],
        bat_info[4]
    );

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

    printk("/proc file created for Lab4 battery\n");

    return 0;
}

void cleanup_module( void ) {
    printk("Lab4 battery module uninstalling\n");

    remove_proc_entry(procfs_name, NULL);

    return;
}