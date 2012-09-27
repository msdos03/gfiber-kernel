#
# Makefile for the Marvell Customer driver
#

ifeq ($(CONFIG_ARCH_FEROCEON),y)
	include $(srctree)/$(MACHINE)/config/mvRules.mk
endif

ifdef CONFIG_MV_HAL_RULES_PATH
include $(srctree)/include/config/auto.conf
include $(srctree)/$(subst ",,$(CONFIG_MV_HAL_RULES_PATH))
endif

CUST_OBJS += 	mv_cust_dev.o \
		mv_cust_mod.o \
		mv_cust_flow_map.o \
		mv_cust_netdev.o \
		mv_cust_sysfs.o
                                
mv_cust-objs := $(CUST_OBJS)
obj-$(CONFIG_MV_CUST) += mv_cust.o

