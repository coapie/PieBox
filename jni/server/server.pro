TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CFLAGS += -I../common -DSQLITE_OMIT_LOAD_EXTENSION

SOURCES += logger.c sqlite3.c db-user.c db-stor.c svr-trans.c svr-mgmt.c svr-stor.c repo.c server.c
SOURCES += daemon.c main.c

HEADERS += ../common/proto.h


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../build-common-Desktop-Debug/release/ -lcommon
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../build-common-Desktop-Debug/debug/ -lcommon
else:unix: LIBS += -L$$PWD/../build-common-Desktop-Debug/ -lcommon

INCLUDEPATH += $$PWD/../build-common-Desktop-Debug
DEPENDPATH += $$PWD/../build-common-Desktop-Debug

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-common-Desktop-Debug/release/libcommon.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-common-Desktop-Debug/debug/libcommon.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../build-common-Desktop-Debug/release/common.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../build-common-Desktop-Debug/debug/common.lib
else:unix: PRE_TARGETDEPS += $$PWD/../build-common-Desktop-Debug/libcommon.a

unix:!macx: LIBS += -lpthread -levent
