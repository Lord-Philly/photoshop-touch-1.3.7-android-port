LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := TTPixelExtensionAndroid
LOCAL_SRC_FILES := compat_stubs.c
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := ane_compat_stub
LOCAL_SRC_FILES := empty_ane.c
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := sibsynclib
LOCAL_SRC_FILES := empty_ane.c
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := air.com.adobe.cc.sync.SyncEngine
LOCAL_SRC_FILES := empty_ane.c
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := AdobeSyncEngineBridge
LOCAL_SRC_FILES := empty_ane.c
include $(BUILD_SHARED_LIBRARY)
