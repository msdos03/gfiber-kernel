#
# Makefile for the Marvell MAC learn module
#
ifeq ($(CONFIG_ARCH_FEROCEON),y)
	include $(srctree)/$(MACHINE)/config/mvRules.mk
endif

ifdef CONFIG_MV_HAL_RULES_PATH
include $(srctree)/include/config/auto.conf
include $(srctree)/$(subst ",,$(CONFIG_MV_HAL_RULES_PATH))
endif

MAC_LEARN_OBJS += mv_mac_learn_mod.o \
		  mv_mac_learn_netdev.o \
		  mv_mac_learn_logic.o \
		  mv_mac_learn_db.o \
		  mv_mac_learn_api.o \
		  mv_mac_learn_sysfs.o

mv_mac_learn-objs := $(MAC_LEARN_OBJS)
obj-$(CONFIG_MV_MAC_LEARN) += mv_mac_learn.o

