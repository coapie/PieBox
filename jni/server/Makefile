
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


LDFLAGS = -static -L$(CROSS_LD) -levent -pthread -ldl

STRIP =$(CROSS)strip

TARGET = PieBox

objs = md5.o rbtree.o logger.o fixbuffer.o fixarray.o fixmap.o cmp.o iarch_mgmt.o iarch_stor.o
objs += db-user.o db-stor.o svr-trans.o svr-mgmt.o svr-stor.o db-repo.o server.o
objs += sharepie.o main.o

vpath %.c ../common


all:$(objs) $(lib-objs) sqlite3.o 
	$(CC) -Wall -o $(TARGET) $(objs) $(lib-objs) sqlite3.o  $(LDFLAGS)
	$(STRIP) -s $(TARGET)


%.o:%.c
	-$(CC) $(CFLAGS) -c -o $@ $<

sqlite3.o:sqlite3.c
	-$(CC) $(CFLAGS) -DSQLITE_OMIT_LOAD_EXTENSION -c -o $@ $<



clean:
	rm -f $(objs) $(TARGET)

