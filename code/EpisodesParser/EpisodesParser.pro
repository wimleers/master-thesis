# Necessary to be able to use the QHostAddress class.
QT += network

SOURCES += \
    Parser.cpp \
    typedefs.cpp

HEADERS += \
    Parser.h \
    typedefs.h \
    QCachingLocale.h

# Disable qDebug() output when in release mode.
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Add a DEBUG define when in debug mode.
CONFIG(debug, debug|release):DEFINES += DEBUG



# Temporary main, for testing purposes.
SOURCES += main.cpp
