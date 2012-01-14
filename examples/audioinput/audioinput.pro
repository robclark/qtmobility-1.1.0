TEMPLATE = app
CONFIG += example

INCLUDEPATH += ../../src/multimedia ../../src/multimedia/audio
include(../examples.pri)

CONFIG += mobility
MOBILITY = multimedia

HEADERS       = audioinput.h

SOURCES       = audioinput.cpp \
                main.cpp

