LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := injector
LOCAL_SRC_FILES := main.cpp injector/injector.cpp injector/npt_injector.cpp memory/ProcessManager.cpp memory/memory.cpp remote_call/remote_call.cpp
LOCAL_CFLAGS := -Wall -Wno-format -O0
LOCAL_LDLIBS := -llog

# Build only for x86_64
LOCAL_MODULE_TARGET_ARCH := arm64-v8a

include $(BUILD_EXECUTABLE)
