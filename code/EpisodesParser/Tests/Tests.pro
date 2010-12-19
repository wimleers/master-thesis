DEPENDPATH += ..
include (../EpisodesParser.pri)

CONFIG += qtestlib
macx {
  CONFIG -= app_bundle
}
TARGET = Tests


HEADERS += TestParser.h
SOURCES += Tests.cpp \
           TestParser.cpp
