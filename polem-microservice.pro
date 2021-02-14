TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        label_processing.cpp \
        main.cpp \
        rest_request_handler.cpp

HEADERS += \
  disk_input.h \
  label_processing.h \
  rest_request_handler.h

unix: LIBS += -L$$PWD/../../../usr/local/lib/ -lpolem-dev
INCLUDEPATH += $$PWD/../../../usr/local/include
DEPENDPATH += $$PWD/../../../usr/local/include
unix:!macx: LIBS += -licuuc

unix: PRE_TARGETDEPS += $$PWD/../../../usr/lib/x86_64-linux-gnu/libpistache.a
INCLUDEPATH += $$PWD/../../../usr/include/pistache
DEPENDPATH += $$PWD/../../../usr/include/pistache
unix: LIBS += -L$$PWD/../../../usr/lib/x86_64-linux-gnu/ -lpistache

unix: LIBS += -lpthread -lssl -lcrypto
