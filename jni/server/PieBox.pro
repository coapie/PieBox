TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CFLAGS += -I. -I../../common

SOURCES += ../common/cmp.c ../common/fixarray.c ../common/fixbuffer.c ../common/fixmap.c
SOURCES += ../common/iarch_mgmt.c ../common/iarch_stor.c ../common/md5.c ../common/rbtree.c

SOURCES += logger.c db-user.c db-stor.c db-repo.c svr-mgmt.c svr-stor.c svr-trans.c
SOURCES +=sqlite3.c server.c sharepie.c main.c

LIBS += -levent -pthread -ldl

