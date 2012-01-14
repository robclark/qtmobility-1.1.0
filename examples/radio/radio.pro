TEMPLATE = app
CONFIG += example

INCLUDEPATH += ../../src/multimedia
include(../examples.pri)

CONFIG += mobility
MOBILITY = multimedia

HEADERS = \
    radio.h
  
SOURCES = \
    main.cpp \
    radio.cpp

symbian: {
    TARGET.CAPABILITY = UserEnvironment WriteDeviceData ReadDeviceData SwEvent
}
