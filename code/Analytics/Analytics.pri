QT += core
QT -= gui

INCLUDEPATH += $${PWD}

SOURCES += \
    $${PWD}/Item.cpp \
    $${PWD}/FPNode.cpp \
    $${PWD}/FPTree.cpp \
    $${PWD}/FPGrowth.cpp\
    $${PWD}/RuleMiner.cpp \
    $${PWD}/Analyst.cpp \
    $${PWD}/Constraints.cpp
HEADERS += \
    $${PWD}/Item.h \
    $${PWD}/FPNode.h \
    $${PWD}/FPTree.h \
    $${PWD}/FPGrowth.h \
    $${PWD}/RuleMiner.h \
    $${PWD}/Analyst.h \
    $${PWD}/Constraints.h

# Disable qDebug() output when in release mode.
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Add a DEBUG define when in debug mode.
CONFIG(debug, debug|release):DEFINES += DEBUG
