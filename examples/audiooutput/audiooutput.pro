TEMPLATE = app
CONFIG += example

INCLUDEPATH += ../../src/multimedia ../../src/multimedia/audio
include(../examples.pri)

CONFIG += mobility
MOBILITY = multimedia

HEADERS       = audiooutput.h

SOURCES       = audiooutput.cpp \
                main.cpp
