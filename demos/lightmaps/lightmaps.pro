TEMPLATE = app
TARGET = lightmaps_with_location

HEADERS = ../../examples/satellitedialog/satellitedialog.h \
            ../../examples/flickrdemo/connectivityhelper.h
SOURCES = lightmaps.cpp \
            ../../examples/satellitedialog/satellitedialog.cpp \
            ../../examples/flickrdemo/connectivityhelper.cpp

QT += network

INCLUDEPATH += ../../src/global \
                ../../src/bearer \
                ../../src/location \
                ../../examples/satellitedialog \
                ../../examples/flickrdemo

include(../demos.pri)

CONFIG += mobility
MOBILITY = location

equals(QT_MAJOR_VERSION, 4) : greaterThan(QT_MINOR_VERSION, 6) {
    # use Bearer Management classes in QtNetwork module
    DEFINES += BEARER_IN_QTNETWORK
} else {
    MOBILITY += bearer
}

symbian|wince* {
    symbian {
        addFiles.sources = nmealog.txt
        DEPLOYMENT += addFiles
        TARGET.CAPABILITY = NetworkServices Location ReadUserData
        TARGET.EPOCHEAPSIZE = 0x20000 0x2000000
    }
    wince*: {
        addFiles.sources = ./nmealog.txt
        addFiles.path = .
        DEPLOYMENT += addFiles
    }
} else {
    logfile.path = $$QT_MOBILITY_DEMOS
    logfile.files = nmealog.txt
    INSTALLS += logfile
}
