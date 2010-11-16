
SOURCES += \
    main.cpp \
    Parser.cpp \
    typedefs.cpp

HEADERS += \
    Parser.h \
    typedefs.h

QT += network

# Disable qDebug() output when in release mode.
CONFIG(release, debug|release):DEFINES += QT_NO_DEBUG_OUTPUT

# Add a DEBUG define when in debug mode.
CONFIG(debug, debug|release):DEFINES += DEBUG
