#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/poll.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/system.h>

#define GLOBALMEM_SIZE 0X10000
#define MEM_CLEAR 0X1
#define GLOBALMEM_MAJOR 250

static int globalmem_major = GLOBALMEM_MAJOR;

struct globalmem_dev {
		struct cdev cdev;
		unsigned char mem[GLOBALMEM_SIZE];
};

struct globalmem_dev dev;
static int globalmem_init(void);
static void globalmem_setup_cdev();

static ssize_t globalmem_read(struct file* filp, char __user* buf, size_t count, loff_t* ppos);
static ssize_t globalmem_write(struct file* filp, char __user* buf, size_t count, loff_t* ppos);
static loff_t globalmem_llseek(struct file* filp, loff_t offset, int orig);
//static int globalmem_ioclt(struct node* inodep, struct file* filp, unsigned int cmd, unsigned long arg);
static int globalmem_ioclt(struct node* inodep, struct file* filp, unsigned int cmd, unsigned long arg)
{
	int i;
	return 0;
}

static const struct file_operations globalmem_fops = {
	.owner = THIS_MODULE,
	.llseek = globalmem_llseek,
	.read = globalmem_read,
	.write = globalmem_write,
	.ioctl = globalmem_ioclt,
};

static int globalmem_init(void)
{
	int result;
	dev_t devno = MKDEV(globalmem_major, 0);

	if ( globalmem_major )
	{
		result = register_chrdev_region(devno, 1, "globalmem");
	}
	else
	{
		result = alloc_chrdev_region(&devno, 0, 1, "globalmem");
		globalmem_major = MAJOR(devno);
	}

	if (result < 0)
	{
		return result;
	}
	
	globalmem_setup_cdev();

	return 0;
}



static void globalmem_setup_cdev()
{
	int err, devno = MKDEV(globalmem_major, 0);

	cdev_init(&dev.cdev, &globalmem_fops);
	dev.cdev.owner = THIS_MODULE;
	err = cdev_add(&dev.cdev, devno, 1);
	if (err)
	{
		printk(KERN_NOTICE "error %d adding globalmem", err);
	}
}

static ssize_t globalmem_read(struct file* filp, char __user* buf, size_t count, loff_t* ppos)
{
	unsigned long p = *ppos;
	int ret = 0;
	return ret;
}
static ssize_t globalmem_write(struct file* filp, char __user* buf, size_t count, loff_t* ppos)
{
	int ret = 0;

	return ret;
}
static loff_t globalmem_llseek(struct file* filp, loff_t offset, int orig)
{
	loff_t ret = 0;

	return ret;
}

//module_init(sixth_drv_init);

//module_exit(sixth_drv_exit);

MODULE_LICENSE("GPL");

