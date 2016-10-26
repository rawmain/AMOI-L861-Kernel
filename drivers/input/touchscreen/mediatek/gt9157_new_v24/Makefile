include $(srctree)/drivers/misc/mediatek/Makefile.custom

# Linux driver folder
ccflags-y += -I$(srctree)/arch/arm/mach-$(MTK_PLATFORM)/$(ARCH_MTK_PROJECT)/touchpanel/gt9157_new_v24/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/gt9157_new_v24/
ccflags-y += -I$(srctree)/drivers/input/touchscreen/mediatek/
ccflags-y += -I$(srctree)/drivers/misc/mediatek/mach/mt6795/L861_stonex/touchpanel/gt9157_new_v24

obj-y	+=  goodix_tool.o
obj-y	+=  gt9xx_driver.o
obj-y	+=  gt9xx_update.o

