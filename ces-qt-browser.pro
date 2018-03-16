TEMPLATE = app

QT += qml quick quickcontrols2 webengine
CONFIG += c++11
static {
    QT += svg
    QTPLUGIN += qtvirtualkeyboardplugin
}


SOURCES += \
    main.cpp \
    quickwindow.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    util.h \
    quickwindow.h

