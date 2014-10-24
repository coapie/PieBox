
CROSS = arm-linux-androideabi-
NDK_PLATFORM=/Applications/ndk/platforms
NDK_TARGET=android-18
NDK_ARCH=arch-arm
CROSS_SYSPATH =$(NDK_PLATFORM)/$(NDK_TARGET)/$(NDK_ARCH)/usr
CROSS_INC = $(CROSS_SYSPATH)/include
CROSS_LD = $(CROSS_SYSPATH)/lib

#CROSS =
#CORSS_SYSPATH =
#CROSS_INC = .
#CROSS_LD = .

CC	= $(CROSS)gcc
CFLAGS	= -Wall -O3 -DTLV_EVBUFFER -I. -I$(CROSS_INC) -I../common
CFLAGS += -DSQLITE_OMIT_LOAD_EXTENSION --sysroot=$(NDK_PLATFORM)/$(NDK_TARGET)/$(NDK_ARCH)

LDFLAGS = -static -L$(CROSS_LD) -levent -pthread

STRIP =$(CROSS)strip

TARGET = iarch

objs = md5.o rbtree.o logger.o fixbuffer.o fixarray.o fixmap.o cmp.o iarch_mgmt.o iarch_stor.o
objs += db-user.o db-stor.o svr-trans.o db-repo.o svr-mgmt.o svr-stor.o  server.o
objs += daemon.o main.o

vpath %.c ../common


%.o:%.c
	-$(CC) $(CFLAGS) -c -o $@ $<


all:$(objs) $(lib-objs) sqlite3.o 
	$(CC) -Wall $(CFLAGS) -o $(TARGET) $(objs) $(lib-objs) sqlite3.o  $(LDFLAGS)
	$(STRIP) -s $(TARGET)


clean:
	rm -f $(objs) $(TARGET)

