
include $(srctree)/drivers/misc/mediatek/Makefile.custom

ifneq ($(CONFIG_ARCH_MTK_PROJECT),)
ARCH_MTK_PROJECT := $(shell echo $(CONFIG_ARCH_MTK_PROJECT))
obj-y   += core/
obj-y   += dct/dct/
endif

ifeq ($(CONFIG_MTK_SENSOR_SUPPORT),y)
ifeq ($(CONFIG_MTK_CM36652_NEW),y)
obj-y	+=  alsps/cm36652-new/
endif

ifeq ($(CONFIG_MTK_STK3X1X_NEW),y)
obj-y	+=  alsps/stk3x1x-new/
endif

ifeq ($(CONFIG_MTK_MPU6050G_NEW),y)
obj-y   +=  accelerometer/mpu6050g-new/
endif

ifeq ($(CONFIG_MTK_KXCJK1013),y)
obj-y   +=  accelerometer/KXCJK1013/
endif

ifeq ($(CONFIG_MTK_MPU6050GY_NEW),y)
obj-y   +=  gyroscope/mpu6050gy-new/
endif

ifeq ($(CONFIG_MTK_MPU3050C_NEW),y)
obj-y   +=  gyroscope/mpu3050c-new/
endif

ifeq ($(CONFIG_MTK_AKM8963_NEW),y)
obj-y	+=  magnetometer/akm8963-new/
endif

ifeq ($(CONFIG_MTK_AKM09911_NEW_61),y)
obj-y	+=  magnetometer/akm09911_new_61/
endif

ifeq ($(CONFIG_MTK_BMP180_NEW),y)
obj-y	+=  barometer/BMP180-new/
endif
endif
obj-y   +=  camera/camera/
obj-y   +=  vibrator/
obj-y   +=  leds/
obj-y   +=  keypad/
obj-y   +=  sound/
obj-y   +=  accdet/

ifeq ($(CONFIG_MTK_HDMI_SUPPORT),y)
obj-y	+=  hdmi/
endif

