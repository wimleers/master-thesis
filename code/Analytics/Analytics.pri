QT += core
QT -= gui

INCLUDEPATH += $${PWD}

SOURCES += \
    $${PWD}/FPNode.cpp \
    $${PWD}/FPTree.cpp \
    $${PWD}/FPGrowth.cpp\
    $${PWD}/RuleMiner.cpp \
    $${PWD}/typedefs.cpp
HEADERS += \
    $${PWD}/FPNode.h \
    $${PWD}/FPTree.h \
    $${PWD}/FPGrowth.h \
    $${PWD}/RuleMiner.h \
    $${PWD}/typedefs.h

# Disable qDebug() output when in release mode.
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Add a DEBUG define when in debug mode.
CONFIG(debug, debug|release):DEFINES += DEBUG
