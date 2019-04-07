LOCAL_PATH := $(call my-dir)/..

include $(CLEAR_VARS)

LOCAL_MODULE := mikmod

MIKMOD_PATH := $(LOCAL_PATH)/..
LOCAL_C_INCLUDES := $(LOCAL_PATH) $(MIKMOD_PATH)/include
LOCAL_LDLIBS += -lOpenSLES
LOCAL_CFLAGS := -DMIKMOD_BUILD -DHAVE_CONFIG_H -Wall
# enable visibility attributes:
LOCAL_CFLAGS += -fvisibility=hidden -DSYM_VISIBILITY
LOCAL_CFLAGS += -O2
#LOCAL_CFLAGS += -Os

LOCAL_SRC_FILES := \
$(MIKMOD_PATH)/drivers/drv_nos.c \
$(MIKMOD_PATH)/drivers/drv_osles.c \
$(MIKMOD_PATH)/loaders/load_669.c \
$(MIKMOD_PATH)/loaders/load_amf.c \
$(MIKMOD_PATH)/loaders/load_asy.c \
$(MIKMOD_PATH)/loaders/load_dsm.c \
$(MIKMOD_PATH)/loaders/load_far.c \
$(MIKMOD_PATH)/loaders/load_gdm.c \
$(MIKMOD_PATH)/loaders/load_gt2.c \
$(MIKMOD_PATH)/loaders/load_imf.c \
$(MIKMOD_PATH)/loaders/load_it.c \
$(MIKMOD_PATH)/loaders/load_m15.c \
$(MIKMOD_PATH)/loaders/load_med.c \
$(MIKMOD_PATH)/loaders/load_mod.c \
$(MIKMOD_PATH)/loaders/load_mtm.c \
$(MIKMOD_PATH)/loaders/load_okt.c \
$(MIKMOD_PATH)/loaders/load_s3m.c \
$(MIKMOD_PATH)/loaders/load_stm.c \
$(MIKMOD_PATH)/loaders/load_stx.c \
$(MIKMOD_PATH)/loaders/load_ult.c \
$(MIKMOD_PATH)/loaders/load_umx.c \
$(MIKMOD_PATH)/loaders/load_uni.c \
$(MIKMOD_PATH)/loaders/load_xm.c \
$(MIKMOD_PATH)/mmio/mmalloc.c \
$(MIKMOD_PATH)/mmio/mmerror.c \
$(MIKMOD_PATH)/mmio/mmio.c \
$(MIKMOD_PATH)/depackers/mmcmp.c \
$(MIKMOD_PATH)/depackers/pp20.c \
$(MIKMOD_PATH)/depackers/s404.c \
$(MIKMOD_PATH)/depackers/xpk.c \
$(MIKMOD_PATH)/posix/strcasecmp.c \
$(MIKMOD_PATH)/playercode/mdreg.c \
$(MIKMOD_PATH)/playercode/mdriver.c \
$(MIKMOD_PATH)/playercode/mdulaw.c \
$(MIKMOD_PATH)/playercode/mloader.c \
$(MIKMOD_PATH)/playercode/mlreg.c \
$(MIKMOD_PATH)/playercode/mlutil.c \
$(MIKMOD_PATH)/playercode/mplayer.c \
$(MIKMOD_PATH)/playercode/munitrk.c \
$(MIKMOD_PATH)/playercode/mwav.c \
$(MIKMOD_PATH)/playercode/npertab.c \
$(MIKMOD_PATH)/playercode/sloader.c \
$(MIKMOD_PATH)/playercode/virtch.c \
$(MIKMOD_PATH)/playercode/virtch2.c \
$(MIKMOD_PATH)/playercode/virtch_common.c

include $(BUILD_SHARED_LIBRARY)
