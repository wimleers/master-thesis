DEPENDPATH += ..
include (../EpisodesParser.pro)

# Ignore the temporary main.
SOURCES -= main.cpp


# Disable EpisodesParser's conditional debug output.
#DEFINES -= DEBUG

CONFIG += debug qtestlib
macx {
  CONFIG -= app_bundle
}
TARGET = Tests


HEADERS += TestParser.h
SOURCES += Tests.cpp \
           TestParser.cpp
