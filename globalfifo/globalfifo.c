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

#define GLOBALMEM_SIZE 0X1000
#define MEM_CLEAR 0X1
#define GLOBALFIFO_MAJOR 250

static int globalfifo_major = GLOBALFIFO_MAJOR;

struct globalfifo_dev {
		struct cdev cdev;
		unsigned char mem[GLOBALMEM_SIZE];
};

struct globalfifo_dev *global_dev;
static int globalfifo_init(void);
static void globalfifo_setup_cdev();

static ssize_t globalfifo_read(struct file* filp, char __user* buf, size_t count, loff_t* ppos);
static ssize_t globalfifo_write(struct file* filp, char __user* buf, size_t count, loff_t* ppos);
static loff_t globalfifo_llseek(struct file* filp, loff_t offset, int orig);
//static int globalfifo_ioclt(struct node* inodep, struct file* filp, unsigned int cmd, unsigned long arg);
static int globalfifo_ioclt(struct node* inodep, struct file* filp, unsigned int cmd, unsigned long arg)
{
	struct globalfifo_dev* dev = filp->private_data;

	switch (cmd)
	{
	case MEM_CLEAR:
		memset(dev->mem, 0, GLOBALMEM_SIZE);
		printk(KERN_INFO "globalfifo is set to zero\n");
		break;
	default:
		return -EINVAL;
		break;
	}

	return 0;
}

int globalfifo_open(struct inode* inode, struct file* filp)
{
	filp->private_data = global_dev;

	return 0;
}

int globalfifo_release(struct inode* indoe, struct file* filp)
{
	return 0;
}

static const struct file_operations globalfifo_fops = {
	.owner = THIS_MODULE,
	.llseek = globalfifo_llseek,
	.read = globalfifo_read,
	.write = globalfifo_write,
	.ioctl = globalfifo_ioclt,
	.open = globalfifo_open,
	.release = globalfifo_release,
};

static int globalfifo_init(void)
{
	int result;
	dev_t devno = MKDEV(globalfifo_major, 0);

	if ( globalfifo_major )
	{
		result = register_chrdev_region(devno, 1, "globalfifo");
	}
	else
	{
		result = alloc_chrdev_region(&devno, 0, 1, "globalfifo");
		globalfifo_major = MAJOR(devno);
	}

	if (result < 0)
	{
		return result;
	}
	
	global_dev = kmalloc(sizeof(struct globalfifo_dev), GFP_KERNEL);
	if (!global_dev)
	{
		result = -ENOMEM;

		goto fail_malloc;
	}
	
	globalfifo_setup_cdev(global_dev, 0);

	return 0;

fail_malloc:
	unregister_chrdev_region(devno, 1);

	return result;
}

static void globalfifo_setup_cdev(struct globalfifo_dev* dev, int index)
{
	int err, devno = MKDEV(globalfifo_major, 0);

	cdev_init(&dev->cdev, &globalfifo_fops);
	dev->cdev.owner = THIS_MODULE;
	err = cdev_add(&dev->cdev, devno, 1);
	if (err)
	{
		printk(KERN_NOTICE "error %d adding globalfifo", err);
	}
}

static ssize_t globalfifo_read(struct file* filp, char __user* buf, size_t count, loff_t* ppos)
{
	unsigned long p = *ppos;
	int ret = 0;
	struct globalfifo_dev *dev = filp->private_data;
	if (p >= GLOBALMEM_SIZE)
	{
		return 0;
	}

	if (count > GLOBALMEM_SIZE - p)
	{
		count = GLOBALMEM_SIZE - p;
	}

	if (copy_to_user(buf, (void*)(dev->mem + p), count))
	{
		ret = -EFAULT;
	}
	else
	{
		*ppos += count;
		ret = count;
		printk(KERN_INFO "read %d bytes from %d\n", count, p);
	}

	return ret;
}
static ssize_t globalfifo_write(struct file* filp, char __user* buf, size_t count, loff_t* ppos)
{
	int ret = 0;
	unsigned long p = *ppos;
	struct globalfifo_dev *dev = filp->private_data;
	if (p >= GLOBALMEM_SIZE)
	{
		return 0;
	}

	if (count > GLOBALMEM_SIZE - p)
	{
		count = GLOBALMEM_SIZE - p;
	}

	if (copy_from_user(dev->mem + p, buf, count))
	{
		ret = -EFAULT;
	}
	else
	{
		*ppos += count;
		ret = count;
	}
	printk(KERN_INFO "write %d bytra from %d\n", count, p);

	return ret;
}
static loff_t globalfifo_llseek(struct file* filp, loff_t offset, int orig)
{
	loff_t ret = 0;
	switch (orig)
	{
	case 0:
		if (offset < 0)
		{
			ret = -EINVAL;
			break;
		}

		if ((unsigned int)offset > GLOBALMEM_SIZE )
		{
			ret = -EINVAL;
			break;
		}

		filp->f_pos = (unsigned int)offset;
		ret = filp->f_pos;

		break;
	case 1:
		if ((filp->f_pos + offset) > GLOBALMEM_SIZE)
		{
			ret = -EINVAL;
			break;
		}

		if ((filp->f_pos + offset) < 0)
		{
			ret = -EINVAL;
			break;
		}

		filp->f_pos += offset;
		ret = filp->f_pos;

		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

void globalfifo_exit(void)
{
	cdev_del(&global_dev->cdev);
	kfree(global_dev);
	unregister_chrdev_region(MKDEV(globalfifo_major, 0), 1);
}

MODULE_AUTHOR("MR.WANG");
MODULE_LICENSE("Dual BSD/GPL");
module_param(globalfifo_major, int, S_IRUGO);

module_init(globalfifo_init);
module_exit(globalfifo_exit);


