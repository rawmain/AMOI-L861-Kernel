#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/workqueue.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/earlysuspend.h>

#include <asm/atomic.h>
#include <asm/uaccess.h>

//#include <mach/irqs.h>

#include <linux/delay.h>

#include <linux/list.h>
#include <linux/string.h>
//#include <linux/leds-mt65xx.h>
#include <cust_eint.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <mach/eint.h>
#include <linux/switch.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>



#define DEVICE_NAME    "door_contact"

/*

#define GPIO_MHALL_EINT_PIN         (GPIO118 | 0x80000000)


#define CUST_EINTF_TRIGGER_RISING     			1    //High Polarity and Edge Sensitive
#define CUST_EINTF_TRIGGER_FALLING    			2    //Low Polarity and Edge Sensitive
#define CUST_EINTF_TRIGGER_HIGH      				4    //High Polarity and Level Sensitive
#define CUST_EINTF_TRIGGER_LOW       				8    //Low Polarity and Level Sensitive

#define CUST_EINT_DEBOUNCE_DISABLE          0
#define CUST_EINT_DEBOUNCE_ENABLE           1

#define CUST_EINT_MHALL_NUM              7
#define CUST_EINT_MHALL_DEBOUNCE_CN      0
#define CUST_EINT_MHALL_TYPE							CUST_EINTF_TRIGGER_FALLING
#define CUST_EINT_MHALL_DEBOUNCE_EN      CUST_EINT_DEBOUNCE_DISABLE

extern int mt_get_gpio_in(unsigned long pin);
extern int mt_set_gpio_pull_enable(unsigned long pin, unsigned long enable);
extern int mt_set_gpio_dir(unsigned long pin, unsigned long dir);
*/
static int g_eint_state = 0;
static int g_door_state = 0;

static struct work_struct door_eint_work;
static struct workqueue_struct * door_eint_workqueue = NULL;

static struct platform_driver door_pdrv;

static struct platform_device door_pdev = {
    .name    =    DEVICE_NAME,
    .id    =    -1,
};

static struct tasklet_struct door_tasklet;

struct device* door_device = NULL; 

static struct switch_dev door_data;

static void door_eint_handler(unsigned long data);

//static DECLARE_TASKLET(door_tasklet, door_eint_handler, 0);
//*****************************************************************//
static ssize_t show_door_state(struct device_driver *driver, char *buf)
{

    return sprintf(buf, "%d\n",g_door_state);
}

static DRIVER_ATTR(door_state, S_IWUSR | S_IRUGO, show_door_state, NULL);
static struct driver_attribute *door_attr_list[] = {
    &driver_attr_door_state,
};


static int door_create_attr(struct device_driver *driver)
{
    int idx, err = 0;
    int num = (int)(sizeof(door_attr_list)/sizeof(door_attr_list[0]));
    if (driver == NULL) {
        return -EINVAL;
    }

    for(idx = 0; idx < num; idx++) {
        if((err = driver_create_file(driver, door_attr_list[idx]))) {
            break;
        }
    }
	
    return err;
}
//*********************************************************************

static void door_eint_work_callback(struct work_struct *work)
{
	mt_eint_mask(CUST_EINT_MHALL_NUM); 
	switch_set_state((struct switch_dev *)&door_data, !g_door_state);

	if(g_eint_state==1)
		{
		
			mt_eint_set_polarity(CUST_EINT_MHALL_NUM, !(1));//   CUST_EINTF_TRIGGER_FALLING
			g_eint_state=0;
			g_door_state=1;
			
		}
	
		else
		{	
		
			mt_eint_set_polarity(CUST_EINT_MHALL_NUM,!(0));//CUST_EINTF_TRIGGER_RISING
			g_eint_state=1;
			g_door_state = 0;
			
		}

	mt_eint_unmask(CUST_EINT_MHALL_NUM);
}


void door_eint_handler(unsigned long data)
{
	char *envp[2];  
	
	mt_eint_mask(CUST_EINT_MHALL_NUM); 
	g_eint_state = mt_get_gpio_in(GPIO_MHALL_EINT_PIN);
	if(g_eint_state==1)
	{
	
		mt_eint_set_polarity(CUST_EINT_MHALL_NUM, !(1));//   CUST_EINTF_TRIGGER_FALLING
		g_eint_state=0;
		g_door_state=1;
		
	}

	else
	{	
	
		mt_eint_set_polarity(CUST_EINT_MHALL_NUM,!(0));//CUST_EINTF_TRIGGER_RISING
		g_eint_state=1;
		g_door_state = 0;
		
	}

 	mt_eint_unmask(CUST_EINT_MHALL_NUM);

	/*
    if(g_door_state)  
        envp[0] = "STATUS=OPEN";  
    else  
        envp[0] = "STATUS=CLOSE";  
    envp[1] = NULL;  
	
    kobject_uevent_env(&door_pdev.dev.kobj, KOBJ_CHANGE, envp);
	*/
	switch_set_state((struct switch_dev *)&door_data, g_door_state);
//	switch_set_state(door_data, g_door_state);

}

static void switch_door_eint_handler(void)
{
   // tasklet_schedule(&door_tasklet);
   queue_work(door_eint_workqueue, &door_eint_work);
}
static DECLARE_TASKLET(door_tasklet, door_eint_handler, 0);

static int door_pdrv_probe(struct platform_device *pdev)
{
	int state = 0;
	s32 ret = 0;
	printk("**********door contact probe!**********\n");

	door_data.name = "door";
    door_data.index = 0;
	ret = switch_dev_register(&door_data);
	 if(ret)
	{
		printk("switch_dev_register returned:%d!\n", ret);
		return 1;
	}

	state = mt_get_gpio_in(GPIO_MHALL_EINT_PIN);
	
	mt_set_gpio_mode(GPIO_MHALL_EINT_PIN, GPIO_MHALL_EINT_PIN_M_EINT);
	mt_eint_set_hw_debounce(GPIO_MHALL_EINT_PIN, CUST_EINT_MHALL_DEBOUNCE_CN);
	

  	mt_set_gpio_dir(GPIO_MHALL_EINT_PIN, GPIO_DIR_IN);
 	mt_set_gpio_pull_enable(GPIO_MHALL_EINT_PIN, GPIO_PULL_DISABLE);
	
	door_eint_workqueue = create_singlethread_workqueue("door_eint");
	INIT_WORK(&door_eint_work, door_eint_work_callback);
	
	if (state == 1)
	{	g_door_state=1;//open
		g_eint_state = 0;
	
		mt_eint_registration(CUST_EINT_MHALL_NUM, CUST_EINTF_TRIGGER_LOW, 
                switch_door_eint_handler, 0);
	
	}
	else
	{	g_door_state=0;//close
		g_eint_state = 1;
	
		mt_eint_registration(CUST_EINT_MHALL_NUM, CUST_EINTF_TRIGGER_HIGH, 
                switch_door_eint_handler, 0);
	
	}
	door_data.state = g_door_state;
   
	switch_set_state((struct switch_dev *)&door_data, g_door_state);
//	switch_set_state(door_data, g_door_state);
	
	if((ret = door_create_attr(&door_pdrv.driver))) 
	{
		printk("create attribute err = %d\n", ret);
	}

    return 0;
}

static int door_pdrv_remove(struct platform_device *pdev)
{
    return 0;
}



static struct platform_driver door_pdrv = {
    .probe    =    door_pdrv_probe,
    .remove    =    door_pdrv_remove,
    .driver    =    {
        .name =    DEVICE_NAME,
        .owner=    THIS_MODULE,
    },
};


static int __init doorcontact_init(void)
{
	int retval=0;
    //platform_device_register(&door_pdev);
#if 1
	retval = platform_device_register(&door_pdev);
	if (retval != 0) {
		printk("huangwb platform device register fail(%d)\n", retval);
		return retval;
	}
	
    platform_driver_register(&door_pdrv);
#endif
    return 0;
}

static void __exit doorcontact_exit(void)
{
    platform_device_unregister(&door_pdev);
    platform_driver_unregister(&door_pdrv);
	return 0;
}

late_initcall(doorcontact_init);
//module_init(doorcontact_init);

module_exit(doorcontact_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("huangwb@amoi.com.cn");
MODULE_DESCRIPTION("DoorContact driver");
