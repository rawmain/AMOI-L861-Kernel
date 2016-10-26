#define pr_fmt(fmt) "["KBUILD_MODNAME"] " fmt
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>
#include <linux/mm.h>
#include <linux/kfifo.h>

#include <linux/firmware.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/of.h>

#include <mach/mt_boot.h>
#include <mach/mt_typedefs.h>
#include <asm/setup.h>

META_COM_TYPE g_meta_com_type = META_UNKNOWN_COM;
unsigned int g_meta_com_id = 0;

struct meta_driver {
    struct device_driver driver;
    const struct platform_device_id *id_table;
};

static struct meta_driver meta_com_type_info = {
    .driver  = {
        .name = "meta_com_type_info",
        .bus = &platform_bus_type,
        .owner = THIS_MODULE,
    },
    .id_table = NULL,
};

static struct meta_driver meta_com_id_info = {
    .driver = {
        .name = "meta_com_id_info",
        .bus = &platform_bus_type,
        .owner = THIS_MODULE,
    },
    .id_table = NULL,
};

#ifdef CONFIG_OF
struct boot_tag_meta_com {
	u32 size;
	u32 tag;
	u32 meta_com_type; /* identify meta via uart or usb */
	u32 meta_com_id;  /* multiple meta need to know com port id */
};
#endif

bool com_is_enable(void)  // usb android will check whether is com port enabled default. in normal boot it is default enabled.
{
    if (get_boot_mode() == NORMAL_BOOT) {
        return false;
	} else {
        return true;
	}
}

void set_meta_com(META_COM_TYPE type, unsigned int id)
{
    g_meta_com_type = type;
    g_meta_com_id = id;
}

META_COM_TYPE get_meta_com_type(void)
{
    return g_meta_com_type;
}

unsigned int get_meta_com_id(void)
{
    return g_meta_com_id;
}

static ssize_t meta_com_type_show(struct device_driver *driver, char *buf)
{
    return sprintf(buf, "%d\n", g_meta_com_type);
}

static ssize_t meta_com_type_store(struct device_driver *driver, const char *buf, size_t count)
{
    /*Do nothing*/
    return count;
}

DRIVER_ATTR(meta_com_type_info, 0644, meta_com_type_show, meta_com_type_store);


static ssize_t meta_com_id_show(struct device_driver *driver, char *buf)
{
    return sprintf(buf, "%d\n", g_meta_com_id);
}

static ssize_t meta_com_id_store(struct device_driver *driver, const char *buf, size_t count)
{
    /*Do nothing*/
    return count;
}

DRIVER_ATTR(meta_com_id_info, 0644, meta_com_id_show, meta_com_id_store);

static int __init create_sysfs(void)
{
    int ret;
    BOOTMODE bm = get_boot_mode();

#ifdef CONFIG_OF
    if (of_chosen) {
        struct boot_tag_meta_com *tags;
        tags = (struct boot_tag_meta_com *)of_get_property(of_chosen, "atag,meta", NULL);
        if (tags) {
            g_meta_com_type = tags->meta_com_type;
            g_meta_com_id = tags->meta_com_id;
            pr_notice("[%s] g_meta_com_type = %d, g_meta_com_id = %d.\n", __func__, g_meta_com_type, g_meta_com_id);
        }
        else
            pr_notice("[%s] No atag,meta found !\n", __func__);
    }
    else
        pr_notice("[%s] of_chosen is NULL !\n", __func__);
#endif

    if(bm == META_BOOT || bm == ADVMETA_BOOT || bm == ATE_FACTORY_BOOT || bm == FACTORY_BOOT) {
        /* register driver and create sysfs files */
        ret = driver_register(&meta_com_type_info.driver);
        if (ret) {
            pr_warn("fail to register META COM TYPE driver\n");
        }
        ret = driver_create_file(&meta_com_type_info.driver, &driver_attr_meta_com_type_info);
        if (ret) {
            pr_warn("fail to create META COM TPYE sysfs file\n");
        }

        ret = driver_register(&meta_com_id_info.driver);
        if (ret) {
            pr_warn("fail to register META COM ID driver\n");
        }
        ret = driver_create_file(&meta_com_id_info.driver, &driver_attr_meta_com_id_info);
        if (ret) {
            pr_warn("fail to create META COM ID sysfs file\n");
        }
    }    
}
//amoi begin
#include <linux/proc_fs.h>
static int amoi_band_info_show(struct seq_file *m, void *v)
{
    #if 1//defined(CONFIG_AMOI_PROJ_L861)

      char strBandInfoShow[]="GSM2/3/5/8,W1/8,LF3/7/20"; 
    #elif defined(CONFIG_AMOI_PROJ_1770)
        char strBandInfoShow[]="GSM2/3/5/8,W1/2/5/8,td34/39,LF1/2/3/5/7/8/12/20,LT38/40/41";  
    #else
        #error
    #endif
    
  	
	seq_printf(m, "%s\n", strBandInfoShow);
	return 0;  
}

static int amoi_band_info_open_proc(struct inode *inode, struct file *file)
{
	return single_open(file, amoi_band_info_show, NULL);
}

static const struct file_operations amoi_band_info_ops = { 
	.open  = amoi_band_info_open_proc, 
	.read  = seq_read,        
};
extern char* saved_command_line; 
unsigned int UbVbat=0;
char strChipIDfromCMD[50]={0};
char strChipIDShow[50]={0};
char strUbVbatfromCMD[50]={0}; 
char *strInfoUbVbat = "UbVbat=";
char *strInfoChipid = "chipid1=";
static void get_commandline_info_bystr(char *strInfoPtr, char *strStorePtr)
{
	char *ptrStart, *ptrEnd;
	ptrStart = strstr(saved_command_line, strInfoPtr);
	if(ptrStart == NULL){
		printk(KERN_ERR "lcg %s line:%d fail\n", __FUNCTION__, __LINE__);
		return ;
	} 
    
    ptrEnd = ptrStart;  
	while(*ptrEnd != ' ' && *ptrEnd != '\0')
		ptrEnd++;  
    strncpy(strStorePtr, ptrStart, ptrEnd-ptrStart);    
    strStorePtr[ptrEnd-ptrStart] = 0;
    printk("lcg get_commandline_info  %s=%s\n", strInfoPtr, strStorePtr);   
}
static void get_chip_info(void)
{
    unsigned int len ;
    get_commandline_info_bystr(strInfoChipid, strChipIDfromCMD);
    len = strlen("chipid1=0x00006752");    
    if(strncmp(strChipIDfromCMD,"chipid1=0x00006752,chipid2=0x00008a00", len) == 0)
    {
        strcpy(strChipIDShow,"MT6752-FDDLTE/TDDLTE/WCDMA/GSM");
    } else {
		strncpy(strChipIDShow, strChipIDfromCMD, strlen(strChipIDfromCMD));	
	} 
}
void get_UbVbat_value(void)
{
    get_commandline_info_bystr(strInfoUbVbat, strUbVbatfromCMD);
    if (strlen(strUbVbatfromCMD)) {
        kstrtoint(strUbVbatfromCMD+strlen(strInfoUbVbat), 0, &UbVbat);         
    }
}
static int mt_cmdinfo_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", strChipIDShow);
    return 0;
}

static int mt_cmdinfo_open_proc(struct inode *inode, struct file *file)
{
	return single_open(file, mt_cmdinfo_show, NULL);
}
static const struct file_operations mt_cmdinfo_ops = {
	.open  = mt_cmdinfo_open_proc, 
	.read  = seq_read,    
};
static int __init boot_mod_init(void)
{
    struct proc_dir_entry *entry = NULL;
    get_chip_info(); 
    get_UbVbat_value();
    entry = proc_create("mt6582chipid", 0666, NULL, &mt_cmdinfo_ops);
    entry = proc_create("mtchipid", 0666, NULL, &mt_cmdinfo_ops);
    if (entry == NULL)
    {
        printk("proc_create mt6582chipid failed\n");
    }
    else
    {
        printk("proc_create mt6582chipid success");
    }
    proc_create("amoi_band_info", S_IRUGO, NULL, &amoi_band_info_ops);
    create_sysfs();
    return 0;
}
//amoi end
static void __exit boot_mod_exit(void)
{
}

module_init(boot_mod_init);
module_exit(boot_mod_exit);
MODULE_DESCRIPTION("MTK Boot Information Querying Driver");
MODULE_LICENSE("GPL");
