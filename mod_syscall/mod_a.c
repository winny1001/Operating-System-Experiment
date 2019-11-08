//mod_a.c

#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
//#include<linux/sched.h>
#include<linux/sched/signal.h>

static char buf[41];
static char buf1[1024];
int init_mymod(void)	//声明是一个模块以及加载时初始化的动作
{
	struct task_struct *p;
	struct file *fp;
	mm_segment_t fs;
	loff_t pos;
	
	printk("Hello Module!\n");
	
	printk("/***************cpu info****************/\n");

	fp = filp_open("/proc/cpuinfo",O_RDONLY,0);//open file
		

	if(IS_ERR(fp)) //check whether the file is correctly open
	{
		printk("create file error\n");
		return -1;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	pos = 79;
	kernel_read(fp,buf,sizeof(buf),&pos);
	printk("%s\n",buf);

	filp_close(fp,NULL);
	set_fs(fs);

	printk("/***************system version****************/\n");

	fp = filp_open("/proc/version",O_RDONLY,0);//open file


        if(IS_ERR(fp)) //check whether the file is correctly open
        {
                printk("create file error\n");
                return -1;
        }

        fs = get_fs();
        set_fs(KERNEL_DS);

        pos = 0;
        kernel_read(fp,buf1,sizeof(buf1),&pos);
        printk("%s\n",buf1);

        filp_close(fp,NULL);
        set_fs(fs);
	
	printk("/***************processes information****************/\n");
   	printk("%-20s%-10s%-15s%-15s%-10s\n","name","pid","time(userM)","time(kernelM)","state");
    	
	for_each_process(p)
    	{
        	printk("%-20s%-10d%-15lld%-15lld%-5ld\n",p->comm,p->pid,(p->utime)/60,(p->stime)/60,p->state);
    	}

	return 0;  
}

void exit_mymod(void)	//卸载模块时的动作
{
	printk("Goodbye Module!\n");
}

module_init(init_mymod); 
module_exit(exit_mymod);

//模块相关的信息，高版本内核必须要写，低版本内核不强制要求
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Test");
MODULE_AUTHOR("WPX");
