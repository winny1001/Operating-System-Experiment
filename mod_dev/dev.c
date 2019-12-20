#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/device.h>
#include <linux/types.h>

#define MAXNUM 100
#define MAJOR_NUM 290

struct dev{
	struct cdev devm;
	struct semaphore sem;
	wait_queue_head_t outq;
	int flag;
	char buffer[MAXNUM + 1];
	char *rd, *wr, *end;
}globalvar;

static struct class *my_class;

int major = MAJOR_NUM;

static ssize_t globalvar_read(struct file *filp, char __user *buf, size_t len, loff_t *ppos)
{
    if(wait_event_interruptible(globalvar.outq, globalvar.flag!=0)) //不可读时 阻塞读进程
    {
        return -ERESTARTSYS;
    }
    
    if(down_interruptible(&globalvar.sem)) //P 操作
    {
        return -ERESTARTSYS;
    }
    
    globalvar.flag = 0;
    
    if(globalvar.rd < globalvar.wr)
        len = min(len,(size_t)(globalvar.wr - globalvar.rd)); //更新读写长度
    else
        len = min(len,(size_t)(globalvar.end - globalvar.rd));
  
    if(copy_to_user(buf,globalvar.rd,len))
    {
        printk(KERN_ALERT"copy failed\n");
        up(&globalvar.sem);
        return -EFAULT;
    }
    
    globalvar.rd = globalvar.rd + len;
    
    if(globalvar.rd == globalvar.end)
        globalvar.rd = globalvar.buffer; //字符缓冲区循环
    
    up(&globalvar.sem); //V 操作
    
    return len;
}

static ssize_t globalvar_write(struct file *filp, const char __user *buf, size_t len, loff_t *ppos)
{
	if(down_interruptible(&globalvar.sem)) //P 操作
    {
        return -ERESTARTSYS;
    }
    
    if(globalvar.rd <= globalvar.wr)
        len = min(len,(size_t)(globalvar.end - globalvar.wr));
    else
        len = min(len,(size_t)(globalvar.rd - globalvar.wr-1));
    
    if(copy_from_user(globalvar.wr,buf,len))
    {
        up(&globalvar.sem); //V 操作
        return -EFAULT;
    }

    globalvar.wr = globalvar.wr + len;
    
    if(globalvar.wr == globalvar.end)
    	globalvar.wr = globalvar.buffer; //循环
    
    up(&globalvar.sem);//V 操作
    
    globalvar.flag = 1; //条件成立,可以唤醒读进程
    
    wake_up_interruptible(&globalvar.outq); //唤醒读进程
    
    return len;
}

static int globalvar_open(struct inode *inode,struct file *filp)
{
	try_module_get(THIS_MODULE); //模块计数加一
    printk("This chrdev is in open\n");
    return(0);
}

static int globalvar_release(struct inode *inode,struct file *filp)
{
    module_put(THIS_MODULE); //模块计数减一
    printk("This chrdev is in release\n");
    return(0);
}

struct file_operations globalvar_fops =
{
	.read = globalvar_read,
	.write = globalvar_write,
	.open = globalvar_open,
	.release = globalvar_release,
};

int globalvar_init(void)
{
	dev_t dev = MKDEV(major, 0);

	int result;

	if(major)
    {
        //静态申请设备编号
        result = register_chrdev_region(dev, 1, "charmem");
    }
    else
    {
        //动态分配设备号
        result = alloc_chrdev_region(&dev, 0, 1, "charmem");
        major = MAJOR(dev);
    }

	if(result < 0)
		return result;

	cdev_init(&globalvar.devm, &globalvar_fops);
	globalvar.devm.owner = THIS_MODULE;
	cdev_add(&globalvar.devm, dev, 1);

	sema_init(&globalvar.sem, 1);
	init_waitqueue_head(&globalvar.outq);
	globalvar.rd = globalvar.buffer;
	globalvar.wr = globalvar.buffer;
	globalvar.end = globalvar.buffer + MAXNUM;
	globalvar.flag = 0;

    my_class = class_create(THIS_MODULE, "chardev0");
    device_create(my_class, NULL, dev, NULL, "chardev0");

    return 0;
}

static void globalvar_exit(void)
{
	device_destroy(my_class, MKDEV(major, 0));
    class_destroy(my_class);
    cdev_del(&globalvar.devm);
    unregister_chrdev_region(MKDEV(major, 0), 1);
}

module_init(globalvar_init);
module_exit(globalvar_exit);
MODULE_LICENSE("GPL");