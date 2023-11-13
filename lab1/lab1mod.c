#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/ktime.h>


#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,6,0)
#define HAVE_PROC_OPS
#endif

static int lab1_show(struct seq_file *m, void *v) {
  //Fetch info for the proccess being run and print it.
  seq_printf(m, "Current Process PCB Information\n");
  seq_printf(m, "Name = %s\n", current->comm);
  seq_printf(m, "PID = %d\n", current->pid);
  seq_printf(m, "PPID = %d\n", task_ppid_nr(current));
  
  //Determine the state on the process, and print this state.
  if (current->state == TASK_RUNNING){
    seq_printf(m, "State = %s\n", "Running");
  }
  else if (current->state == TASK_STOPPED){
    seq_printf(m, "State = %s\n", "Stopped");
  }
  else{
    seq_printf(m, "State = %s\n", "Waiting");
  }

  //Fetch id's and print them.
  seq_printf(m, "Real UID = %d\n", current->cred->uid);
  seq_printf(m, "Effective UID = %d\n", current->cred->euid);
  seq_printf(m, "Saved UID = %d\n", current->cred->suid);
  seq_printf(m, "Real GID = %d\n", current->cred->gid);
  seq_printf(m, "Effective GID = %d\n", current->cred->egid);
  seq_printf(m, "Saved GID = %d\n", current->cred->sgid);

  return 0;
}

static int lab1_open(struct inode *inode, struct  file *file) {
  //Open proc file
  return single_open(file, lab1_show, NULL);
}

#ifdef HAVE_PROC_OPS
static const struct proc_ops lab1_fops = {
  .proc_open = lab1_open,
  .proc_read = seq_read,
  .proc_lseek = seq_lseek,
  .proc_release = single_release,
};
#else
static const struct file_operations lab1_fops = {
  .owner = THIS_MODULE,
  .open = lab1_open,
  .read = seq_read,
  .llseek = seq_lseek,
  .release = single_release,
};
#endif

static int __init lab1_init(void) {
  //Error case for creating the proc file
  if (!proc_create("lab1", 0, NULL, &lab1_fops)) {
    return -ENOMEM;
    }
  //Create proc file
  proc_create("lab1", 0, NULL, &lab1_fops);
  printk(KERN_INFO "lab1mod in\n");
  return 0;
}

static void __exit lab1_exit(void) {
  remove_proc_entry("lab1", NULL);
  printk(KERN_INFO "lab1mod out\n");
}

MODULE_LICENSE("GPL");
module_init(lab1_init);
module_exit(lab1_exit);
