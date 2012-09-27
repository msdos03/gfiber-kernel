# kernel sources 
srctree  := $(shell /bin/pwd)

ifndef KSRC
KSRC  := $(srctree)/../../../..
endif

include $(KSRC)/.config

ifdef CONFIG_CPU_BIG_ENDIAN
CFLAGS  += -DCONFIG_CPU_BIG_ENDIAN
endif

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
AR := $(CROSS_COMPILE)ar

ifeq ($(CONFIG_ZARLINK_SLIC_SUPPORT),y)
TOOL := mv_voice_tool.c

ifeq ($(CONFIG_MV_TDM_USE_DCO),y)
CFLAGS += -DMV_TDM_USE_DCO
endif

ifeq ($(CONFIG_ZARLINK_SLIC_VE880),y)
	LIBS := zarlink_880.a
	PRE_COMPILED_LIB := zarlink_880.lib
	ZARLINK_BASE_PATH = api_lib
	ZARLINK_COMMON_SRC_PATH = $(ZARLINK_BASE_PATH)/common
	ZARLINK_ARCH_MARVELL_PATH = arch_marvell
	ZARLINK_VP880API_SRC_PATH = $(ZARLINK_BASE_PATH)/vp880_api

	CFLAGS += -DZARLINK_SLIC_VE880
	CFLAGS += -I$(ZARLINK_BASE_PATH)/includes \
			-I$(ZARLINK_ARCH_MARVELL_PATH) \
			-I$(ZARLINK_BASE_PATH)/vp880_api \
			-I$(KSRC)/arch/arm/plat-feroceon/mv_drivers_lsp/mv_phone

	TARGETS =  vp_api_common.o vp_api_config.o vp_api_control.o vp_api_cslac_seq.o \
			vp_api_init.o vp_api_query.o vp_api_test.o vp_debug.o vp880_abs_calibration.o \
			vp880_calibration_common.o vp880_control_common.o vp880_fxo_control.o vp880_fxs_control.o \
			vp880_init.o vp880_linestate_control.o vp880_lp_control.o vp880_query.o vp880_seq.o vp880_tracker_calibration.o

	SOURCES =  $(ZARLINK_COMMON_SRC_PATH)/vp_api_common.c $(ZARLINK_COMMON_SRC_PATH)/vp_api_config.c \
			$(ZARLINK_COMMON_SRC_PATH)/vp_api_control.c $(ZARLINK_COMMON_SRC_PATH)/vp_api_cslac_seq.c \
			$(ZARLINK_COMMON_SRC_PATH)/vp_api_init.c $(ZARLINK_COMMON_SRC_PATH)/vp_api_query.c \
			$(ZARLINK_COMMON_SRC_PATH)/vp_api_test.c $(ZARLINK_COMMON_SRC_PATH)/vp_debug.c \
			$(ZARLINK_VP880API_SRC_PATH)/vp880_abs_calibration.c $(ZARLINK_VP880API_SRC_PATH)/vp880_calibration_common.c \
			$(ZARLINK_VP880API_SRC_PATH)/vp880_control_common.c $(ZARLINK_VP880API_SRC_PATH)/vp880_fxo_control.c \
			$(ZARLINK_VP880API_SRC_PATH)/vp880_fxs_control.c $(ZARLINK_VP880API_SRC_PATH)/vp880_init.c \
			$(ZARLINK_VP880API_SRC_PATH)/vp880_linestate_control.c $(ZARLINK_VP880API_SRC_PATH)/vp880_lp_control.c \
			$(ZARLINK_VP880API_SRC_PATH)/vp880_query.c $(ZARLINK_VP880API_SRC_PATH)/vp880_seq.c \
			$(ZARLINK_VP880API_SRC_PATH)/vp880_tracker_calibration.c
	
	ARCHSRC = $(ZARLINK_ARCH_MARVELL_PATH)/sys_service.c $(ZARLINK_ARCH_MARVELL_PATH)/vp_hal.c

	PROFILE = profile_88266.c
endif
ifeq ($(CONFIG_ZARLINK_SLIC_VE792),y)
	LIBS := zarlink_792.a
	PRE_COMPILED_LIB := zarlink_792.lib
	ZARLINK_BASE_PATH = vp792_api_lib
	ZARLINK_COMMON_SRC_PATH = $(ZARLINK_BASE_PATH)/common
	ZARLINK_ARCH_MARVELL_PATH = arch_marvell
	ZARLINK_VP792API_SRC_PATH = $(ZARLINK_BASE_PATH)/vp792_api

	CFLAGS += -DZARLINK_SLIC_VE792
	CFLAGS += -I$(ZARLINK_BASE_PATH)/includes \
			-I$(ZARLINK_ARCH_MARVELL_PATH) \
			-I$(ZARLINK_BASE_PATH)/vp792_api \
			-I$(KSRC)/arch/arm/plat-feroceon/mv_drivers_lsp/mv_phone

	TARGETS = vp_api_config.o vp_api_common.o vp_api_init.o vp_api_query.o vp_api_control.o vp_debug.o \
			vp792_common.o vp792_config.o vp792_control.o vp792_init.o vp792_query.o vp792_firmware.o

	SOURCES =  $(ZARLINK_COMMON_SRC_PATH)/vp_api_config.c  $(ZARLINK_COMMON_SRC_PATH)/vp_api_common.c \
			$(ZARLINK_COMMON_SRC_PATH)/vp_api_init.c  $(ZARLINK_COMMON_SRC_PATH)/vp_api_query.c \
			$(ZARLINK_COMMON_SRC_PATH)/vp_api_control.c $(ZARLINK_COMMON_SRC_PATH)/vp_debug.c \
			$(ZARLINK_VP792API_SRC_PATH)/vp792_common.c $(ZARLINK_VP792API_SRC_PATH)/vp792_config.c \
			$(ZARLINK_VP792API_SRC_PATH)/vp792_control.c $(ZARLINK_VP792API_SRC_PATH)/vp792_init.c \
			$(ZARLINK_VP792API_SRC_PATH)/vp792_query.c $(ZARLINK_VP792API_SRC_PATH)/vp792_firmware.c

	ARCHSRC = $(ZARLINK_ARCH_MARVELL_PATH)/sys_service.c $(ZARLINK_ARCH_MARVELL_PATH)/vp_hal.c $(ZARLINK_ARCH_MARVELL_PATH)/Le71HP0410G_init.c

 	PROFILE = profile_79238.c
endif

all:
	cp libs/$(PRE_COMPILED_LIB) $(LIBS) 
ifneq ($(CONFIG_MV_PHONE_USE_SLIC_LIBS),y)
	rm -f *.o $(LIBS) mv_voice_tool
	$(CC) $(CFLAGS) -static -c $(SOURCES)
	$(AR) rcs $(LIBS) $(TARGETS)
	rm -f *.o 
endif	
	$(CC) $(CFLAGS) -static $(ARCHSRC) $(PROFILE) $(TOOL) -L. $(LIBS) -o mv_voice_tool -lrt -lpthread

clean:
	rm -f *.o $(LIBS) mv_voice_tool
	@for subdir in $(SUBDIRS); do \
                (cd $$subdir && $(MAKE) $@) || exit 1; \
        done

endif #CONFIG_ZARLINK_SLIC_SUPPORT

