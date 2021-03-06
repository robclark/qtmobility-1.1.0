QT += network

TARGET = GeoServiceDemo
TEMPLATE = app

INCLUDEPATH += ../../src/location \
               ../../src/bearer \
               ../../src/location/maps

SOURCES += main.cpp\
        routepresenter.cpp \
        placepresenter.cpp \
        tabbedwindow.cpp \
        routetab.cpp \
        geocodingtab.cpp \
        revgeocodingtab.cpp \
        servicestab.cpp 

HEADERS  += routepresenter.h \
        placepresenter.h \
        tabbedwindow.h \
        routetab.h \
        geocodingtab.h \
        revgeocodingtab.h \
        servicestab.h

include(../examples.pri)

CONFIG += mobility
MOBILITY = location bearer
#contains(QT_MAJOR_VERSION, 4):lessThan(QT_MINOR_VERSION, 7){
#    MOBILITY += bearer
#    INCLUDEPATH += ../../src/bearer
#}

symbian: {
    TARGET.CAPABILITY = Location \
                        NetworkServices \
                        ReadUserData \
                        WriteUserData \
                        ReadDeviceData \
                        WriteDeviceData
}
