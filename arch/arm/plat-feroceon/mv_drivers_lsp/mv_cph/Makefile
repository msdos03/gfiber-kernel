#
# Makefile for the Marvell CPU Packet Handler
#

ifeq ($(CONFIG_ARCH_FEROCEON),y)
	include $(srctree)/$(MACHINE)/config/mvRules.mk
endif

ifdef CONFIG_MV_HAL_RULES_PATH
include $(srctree)/include/config/auto.conf
include $(srctree)/$(subst ",,$(CONFIG_MV_HAL_RULES_PATH))
endif

CPH_OBJS += mv_cph_api.o \
		mv_cph_app.o \
		mv_cph_db.o \
		mv_cph_dev.o \
		mv_cph_flow.o \
		mv_cph_infra.o \
		mv_cph_mod.o \
		mv_cph_netdev.o \
		mv_cph_sysfs.o
                                
mv_cph-objs := $(CPH_OBJS)
obj-$(CONFIG_MV_CPH) += mv_cph.o
