#
# Makefile for the Marvell Gpon driver
#

ifeq ($(CONFIG_ARCH_FEROCEON),y)
	include $(srctree)/$(MACHINE)/config/mvRules.mk
endif

ifdef CONFIG_MV_HAL_RULES_PATH
include $(srctree)/include/config/auto.conf
include $(srctree)/$(subst ",,$(CONFIG_MV_HAL_RULES_PATH))
endif

PON_OBJ += ../../mv_hal/pon/mvPonOnuRegs.o \
	../../mv_hal/pon/mvPonOnuMac.o \
	plat/ponOnuBoard.o \
	plat/ponOnuLnxKsOs.o \
	perf/ponOnuLnxKsMI.o

GPON_OBJS += core/gpon/gponOnuAlloc.o \
	core/gpon/gponOnuAlrm.o \
	core/gpon/gponOnuApi.o \
	core/gpon/gponOnuBer.o \
        core/gpon/gponOnuDb.o \
	core/gpon/gponOnuInit.o \
	core/gpon/gponOnuIsr.o \
	core/gpon/gponOnuMngr.o \
	core/gpon/gponOnuMngrStateMachine.o \
        core/gpon/gponOnuPm.o \
	core/gpon/gponOnuSrvc.o \
	core/gpon/gponOnuGoogle.o \
	perf/gpon/gponOnuLnxKsUI.o \
	perf/gpon/gponOnuLnxKsMI.o \
	plat/gpon/gponOnuBoard.o \
	plat/gpon/gponOnuLnxKsOs.o

EPON_OBJS += core/epon/eponOnuAlrm.o \
	core/epon/eponOnuApi.o \
        core/epon/eponOnuDb.o \
	core/epon/eponOnuInit.o \
	core/epon/eponOnuIsr.o \
	core/epon/eponOnuMngr.o \
        core/epon/eponOnuPm.o \
	perf/epon/eponOnuLnxKsUI.o \
	perf/epon/eponOnuLnxKsMI.o \
	plat/epon/eponOnuBoard.o \
	plat/epon/eponOnuLnxKsOs.o

obj-$(CONFIG_MV_INCLUDE_PON) += mv_pon.o
mv_pon-y := $(PON_OBJ)
mv_pon-$(CONFIG_MV_GPON) += $(GPON_OBJS)
mv_pon-$(CONFIG_MV_EPON) += $(EPON_OBJS)

