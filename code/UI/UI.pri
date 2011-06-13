QT += core gui

INCLUDEPATH += $${PWD}

SOURCES += \
    $${PWD}/MainWindow.cpp
HEADERS += \
    $${PWD}/MainWindow.h

# Disable qDebug() output when in release mode.
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Add a DEBUG define when in debug mode.
CONFIG(debug, debug|release):DEFINES += DEBUG

FORMS += \
    UI/mainwindow.ui
