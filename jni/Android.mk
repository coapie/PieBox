LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CFLAGS :=  -I$(LOCAL_PATH)/common  -DANDROID_NDK
LOCAL_LDLIBS += -levent -llog

LOCAL_MODULE    := sharepie
LOCAL_SRC_FILES := NativeDaemons.c
LOCAL_SRC_FILES += common/cmp.c common/md5.c common/rbtree.c  common/fixbuffer.c common/fixarray.c common/fixmap.c common/iarch_mgmt.c common/iarch_stor.c
LOCAL_SRC_FILES += server/db-stor.c server/db-user.c server/db-repo.c server/sharepie.c server/sqlite3.c
LOCAL_SRC_FILES += server/logger.c server/server.c server/svr-mgmt.c server/svr-stor.c server/svr-trans.c


include $(BUILD_SHARED_LIBRARY)

ifeq ($(TARGET_PLATFORM),android-18)
    LOCAL_CFLAGS   += -DANDROID18
endif

