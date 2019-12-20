#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/init.h>
#include<linux/uaccess.h>
#include<linux/semaphore.h>
#include<linux/wait.h>

#define MAJOR_NUM 290

struct dev{
	struct semaphore sem;
	struct dev
}

static int first_drv_open(struct inode *inode, struct file *file)
{
	printk("first_drv_open...\r\n");
	return 0;
}

static ssize_t first_drv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	printk("first_drv_write...\r\n");
	return 0;
}

static ssize_t first_drv_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{

}

static int first_drv_release(struct inode *inode, struct file *file)
{

}

static struct file_operations first_drv_fops = 
{
	.owner = THIS_MODULE,
	.open = first_drv_open,
	.write = first_drv_write,
	.read = first_drv_read,
	.release = first_drv_release,
};

int first_drv_init(void)
{
	register_chrdev(MAJOR_NUM, "first_drv", &first_drv_fops);
	return 0;
}

void first_drv_exit(void)
{
	unregister_chrdev(MAJOR_NUM, "first_drv");
}

module_init(first_drv_init);
module_exit(first_drv_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("WPX");
