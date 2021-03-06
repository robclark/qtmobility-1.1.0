TEMPLATE = app
TARGET = weatherinfo_with_location

HEADERS = ../../examples/satellitedialog/satellitedialog.h \
            ../../examples/flickrdemo/connectivityhelper.h
SOURCES = weatherinfo.cpp \
            ../../examples/satellitedialog/satellitedialog.cpp \
            ../../examples/flickrdemo/connectivityhelper.cpp

RESOURCES = weatherinfo.qrc
QT += network svg

include(../demos.pri)

CONFIG += mobility
MOBILITY = location

equals(QT_MAJOR_VERSION, 4) : greaterThan(QT_MINOR_VERSION, 6) {
    # use Bearer Management classes in QtNetwork module
    DEFINES += BEARER_IN_QTNETWORK
} else {
    MOBILITY += bearer
}

INCLUDEPATH += ../../src/global \
                ../../src/bearer \
                ../../src/location \
                ../../examples/satellitedialog \
                ../../examples/flickrdemo

symbian {
    symbian {
        addFiles.sources = nmealog.txt
        DEPLOYMENT += addFiles
        TARGET.CAPABILITY += Location \
                NetworkServices \
                ReadUserData
    }
    wince* {
        addFiles.sources = ./nmealog.txt
        addFiles.path = .
        DEPLOYMENT += addFiles
    }
} else {
    logfile.path = $$QT_MOBILITY_DEMOS
    logfile.files = nmealog.txt
    INSTALLS += logfile
}
