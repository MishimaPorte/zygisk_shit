LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libwhale
LOCAL_SRC_FILES := lib/$(TARGET_ARCH_ABI)/libwhale.so
include $(PREBUILT_SHARED_LIBRARY)

