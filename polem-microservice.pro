TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        label_processing.cpp \
        main.cpp

HEADERS += \
  label_processing.h

unix: LIBS += -L$$PWD/../../../usr/local/lib/ -lpolem-dev
INCLUDEPATH += $$PWD/../../../usr/local/include
DEPENDPATH += $$PWD/../../../usr/local/include
unix:!macx: LIBS += -licuuc

unix: PRE_TARGETDEPS += $$PWD/../../../usr/lib/x86_64-linux-gnu/libpistache.a
INCLUDEPATH += $$PWD/../../../usr/include/pistache
DEPENDPATH += $$PWD/../../../usr/include/pistache
unix: LIBS += -L$$PWD/../../../usr/lib/x86_64-linux-gnu/ -lpistache

unix: LIBS += -lpthread -lssl -lcrypto
