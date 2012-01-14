TEMPLATE = lib
CONFIG += plugin
INCLUDEPATH += ../../src/serviceframework
PLUGIN_TYPE = serviceframework
HEADERS += note.h \
           notesmanagerplugin.h \
           notesmanager.h
SOURCES += notesmanagerplugin.cpp \
           notesmanager.cpp
QT += sql
TARGET = serviceframework_notesmanagerplugin

contains(QT_CONFIG, declarative):DEFINES += DECLARATIVE

include(../examples.pri)
CONFIG += mobility
MOBILITY = serviceframework

symbian {
    load(data_caging_paths)
    pluginDep.sources = serviceframework_notesmanagerplugin.dll
    pluginDep.path = $$QT_PLUGINS_BASE_DIR    
    DEPLOYMENT += pluginDep

    xmlautoimport.path = /private/2002AC7F/import/
    xmlautoimport.sources = notesmanagerservice.xml
    DEPLOYMENT += xmlautoimport

    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.CAPABILITY = ALL -TCB
}

xml.path = $$QT_MOBILITY_EXAMPLES/xmldata
xml.files = notesmanagerservice.xml
INSTALLS += xml
