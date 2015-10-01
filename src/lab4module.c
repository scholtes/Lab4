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
 * The module does not report a battery status every second, but rather
 * reports the battery status to a /proc file whenever the /proc file
 * is read. In this manner, the module reports lazily and is CPU friendly.
 *
 * Demonstration:
 * 
 *     usr$ cat /proc/lab4battery
 *     0 50000 0 0  50000
 * 
 * The order of the parameters given is power unit, last full charge, state
 * discharge rate, and remaining capacity. So the values above mean:
 * 
 *    -Capacity reported in milliwatt hours
 *    -Last full charge was 50000 mWh
 *    -Charger plugged in
 *    -Current discharge rate = 0 (because charging)
 *    -Current charge is 50000 mWh (fully charged)
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
    static int ret[5] = { 0, 0, 0, 0, 0 };

    acpi_status status;
    acpi_handle handle;
    union acpi_object *result;
    struct acpi_buffer buffer1 = {ACPI_ALLOCATE_BUFFER, NULL};
    struct acpi_buffer buffer2 = {ACPI_ALLOCATE_BUFFER, NULL};
    int bst_state, bst_rate, bst_rem, bif_power_unit, bif_last_fc;

    status = acpi_get_handle(NULL, (acpi_string)"\\_SB_.PCI0.BAT0", &handle);

    status = acpi_evaluate_object(handle, "_BST", NULL, &buffer1);
    result = buffer1.pointer;

    if(result) {
        bst_state = result->package.elements[0].integer.value;
        bst_rate = result->package.elements[1].integer.value;
        bst_rem = result->package.elements[2].integer.value;
        ret[2] = bst_state;
        ret[3] = bst_rate;
        ret[4] = bst_rem;
        kfree(result);
    }

    status = acpi_evaluate_object(handle, "_BIF", NULL, &buffer2);
    result = buffer2.pointer;

    if(result) {
        bif_power_unit = result->package.elements[0].integer.value;
        bif_last_fc = result->package.elements[2].integer.value;
        ret[0] = bif_power_unit;
        ret[1] = bif_last_fc;
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