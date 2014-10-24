
CROSS = arm-none-linux-gnueabi-
CROSS_SYSPATH = /opt/linux-arm-system
CROSS_INC = $(CROSS_SYSPATH)/include
CROSS_LD = $(CROSS_SYSPATH)/lib

CROSS =
CORSS_SYSPATH =
CROSS_INC = .
CROSS_LD = .

CC	= $(CROSS)gcc
CFLAGS	= -Wall -O3 -I. -I$(CROSS_INC) -I../common
#CFLAGS += -DSQLITE_OMIT_LOAD_EXTENSION

LDFLAGS = -static -L$(CROSS_LD)

STRIP =$(CROSS)strip

TARGET = PieBoxCli

#objs = tlv.o array.o hashset.o
objs += md5.o cmp.o iarch_mgmt.o iarch_stor.o fixbuffer.o fixarray.o
objs += cli-trans.o cli-mgmt.o cli-stor.o session.o main.o

tlv-objs = tlv-test.o tlv.o

test-objs = $(tlv-objs)

vpath %.c ../common


%.o:%.c
	-$(CC) $(CFLAGS) -c -o $@ $<


all:$(objs) $(lib-objs)
	$(CC) -Wall -o $(TARGET) $(objs) $(lib-objs) $(LDFLAGS)
	$(STRIP) -s $(TARGET)


clean:
	rm -f $(objs) $(TARGET)

