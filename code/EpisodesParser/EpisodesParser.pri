# The network module is necessary to be able to use the QHostAddress class.
QT += core network
QT -= gui

INCLUDEPATH += \
    $${PWD} \
    $${PWD}/QCachingLocale \
    $${PWD}/QBrowsCap
DEPENDPATH += \
    $${PWD}/QCachingLocale \
    $${PWD}/QBrowsCap

include("QBrowsCap/QBrowsCap.pri")

SOURCES += \
    $${PWD}/Parser.cpp \
    $${PWD}/typedefs.cpp

HEADERS += \
    $${PWD}/Parser.h \
    $${PWD}/typedefs.h \
    $${PWD}/QCachingLocale/QCachingLocale.h

# Disable qDebug() output when in release mode.
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Add a DEBUG define when in debug mode.
CONFIG(debug, debug|release):DEFINES += DEBUG
