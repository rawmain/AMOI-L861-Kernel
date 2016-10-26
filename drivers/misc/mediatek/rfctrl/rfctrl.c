#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/delay.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/workqueue.h>
#include <linux/spi/spi.h>
#include <linux/gpio.h>
#include <linux/poll.h>
#include <linux/syscalls.h>
#include <linux/fcntl.h>
#include <linux/cdev.h>
#include <mach/mt_typedefs.h>
#include <mach/mt_gpio.h>
#include <linux/miscdevice.h>




#define RFCTRL_IOC_MAGIC 'Z'

//#define RFCTRL_STATE_SWITCH0 _IO(RFCTRL_IOC_MAGIC,4)
#define RFCTRL_STATE_SWITCH1 _IO(RFCTRL_IOC_MAGIC,0)
#define RFCTRL_STATE_SWITCH2 _IO(RFCTRL_IOC_MAGIC,1)
//#define RFCTRL_STATE_SWITCH3 _IO(RFCTRL_IOC_MAGIC,2)
//#define RFCTRL_STATE_SWITCH4 _IO(RFCTRL_IOC_MAGIC,3)

static int GLOBAL_RFCTRLDEV_MAJOR = 0;
static int GLOBAL_RFCTRLDEV_MINOR = 0;

//#define SELECT_ENABLE   (GPIO84 |0x80000000)    // EN
//#define GPIO_SWITCH2    (GPIO86 |0x80000000)    // CT2
#define GPIO_SWITCH1    (GPIO49 |0x80000000)    // CT1

#define RFCTRL_GPIO_OUTPUT(pin,level)	do{\
                                            	mt_set_gpio_mode(pin, 0);\
                                            	mt_set_gpio_dir(pin, GPIO_DIR_OUT);\
                                            	mt_set_gpio_out(pin, level);\		
                                            }while(0)


static ssize_t rfctrl_write(struct file *filp, const char __user *buf,
							 size_t count, loff_t *f_pos){
	return count;

}
static ssize_t rfctrl_read(struct file *filp, char __user *buf, 
								size_t count, loff_t *f_pos){
	return count;


}

static int rfctrl_open(struct inode *inode, struct file *filp){
	int ret = 0;
//	RFCTRL_GPIO_OUTPUT(SELECT_ENABLE,1);
	ret = nonseekable_open(inode, filp);
	printk("rfctrl_open error \n");
	return ret;
	
}

static int rfctrl_release(struct inode *inode, struct file *filp){
//	RFCTRL_GPIO_OUTPUT(SELECT_ENABLE,0);
	return 0;
}

static long rfctrl_ioctl(struct file *filp, unsigned int cmd, unsigned long arg){
	
	int			retval = 0;
    u32			tmp;
	int			err = 0;
    if (_IOC_TYPE(cmd) != RFCTRL_IOC_MAGIC)
        return -ENOTTY;
	
    if (_IOC_DIR(cmd) & _IOC_READ)
    {
        err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));
    }
    else if (_IOC_DIR(cmd) & _IOC_WRITE)
    {
        err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));
    }

    if (err)
    {
        printk("tpd: access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));
        return -EFAULT;
    }
	retval = __get_user(tmp, (u8 __user *)arg);
	printk("----rfctrl enable = %d\n",tmp);
	if(retval != 0)
	{
		return -EFAULT;
	}
	switch(cmd){
		case RFCTRL_STATE_SWITCH1:
		{
//			RFCTRL_GPIO_OUTPUT(SELECT_ENABLE,tmp);
			RFCTRL_GPIO_OUTPUT(GPIO_SWITCH1,0);
//			RFCTRL_GPIO_OUTPUT(GPIO_SWITCH2,0);
			printk("----RFCTRL_STATE_SWITCH1 enable = %d\n",tmp);
		}
		break;
		case RFCTRL_STATE_SWITCH2:
		{
//			RFCTRL_GPIO_OUTPUT(SELECT_ENABLE,tmp);
			RFCTRL_GPIO_OUTPUT(GPIO_SWITCH1,1);
//			RFCTRL_GPIO_OUTPUT(GPIO_SWITCH2,1);
			printk("----RFCTRL_STATE_SWITCH2 enable = %d\n",tmp);
		}
		break;
#if 0
		case RFCTRL_STATE_SWITCH3:
		{
			RFCTRL_GPIO_OUTPUT(SELECT_ENABLE,tmp);
			RFCTRL_GPIO_OUTPUT(GPIO_SWITCH1,1);
			RFCTRL_GPIO_OUTPUT(GPIO_SWITCH2,0);
			printk("----RFCTRL_STATE_SWITCH3 enable = %d\n",tmp);
		}
		break;
		case RFCTRL_STATE_SWITCH4:
		{
			RFCTRL_GPIO_OUTPUT(SELECT_ENABLE,tmp);
			RFCTRL_GPIO_OUTPUT(GPIO_SWITCH1,1);
			RFCTRL_GPIO_OUTPUT(GPIO_SWITCH2,1);
			printk("----RFCTRL_STATE_SWITCH4 enable = %d\n",tmp);
		}
#endif
		break;
		default:
			printk("----default enable = 9\n");
//			RFCTRL_GPIO_OUTPUT(SELECT_ENABLE,0);
			RFCTRL_GPIO_OUTPUT(GPIO_SWITCH1,0);
//			RFCTRL_GPIO_OUTPUT(GPIO_SWITCH2,0);
		break;
	}
}

static const struct file_operations rfctrl_fops = {
//    .owner =	THIS_MODULE,
//    .write =	rfctrl_write,
//    .read =		rfctrl_read,

    .unlocked_ioctl = rfctrl_ioctl,
    .open =		rfctrl_open,
    .release =	rfctrl_release,
};

//static struct cdev devno;
static struct miscdevice rfctrl_misc_device =
{
    .minor = MISC_DYNAMIC_MINOR,
    .name = "rfctrl",
    .fops = &rfctrl_fops,
};

static int __init rfctrl_init(void){
	#if 0
    int status = 0;
	dev_t dev_num;
	printk("rfctrl init begin\n");
	status = alloc_chrdev_region(&dev_num,0,1,"rfctrl");
    if (status < 0){
        return status;
    }
	GLOBAL_RFCTRLDEV_MAJOR = MAJOR(dev_num);
	GLOBAL_RFCTRLDEV_MINOR = MINOR(dev_num);

	cdev_init(&devno,&rfctrl_fops); // 初始化设备
	devno.owner=THIS_MODULE;
	devno.ops=&rfctrl_fops;
	status = cdev_add(&devno,MKDEV(GLOBAL_RFCTRLDEV_MAJOR,0),1);
	if(status){
		printk("rfctrl error\n");
	}
	#endif
	if (misc_register(&rfctrl_misc_device))
    {
		printk("rfctrl_misc_device register failed\n");
    }
	
	printk("rfctrl init ending\n");

}

static int __exit rfctrl_exit(void){
#if 0
	cdev_del(&devno);
 	unregister_chrdev_region(MKDEV(GLOBAL_RFCTRLDEV_MAJOR,0),1);
#endif

}

module_init(rfctrl_init);
module_exit(rfctrl_exit);


MODULE_AUTHOR("AMOI rfctrl");
MODULE_DESCRIPTION("rfctrl driver");
MODULE_LICENSE("GPL");


