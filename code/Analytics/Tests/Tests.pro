DEPENDPATH += ..
include (../Analytics.pri)

CONFIG += qtestlib
macx {
  CONFIG -= app_bundle
}
TARGET = Tests


HEADERS += TestFPTree.h \
           TestFPGrowth.h
SOURCES += Tests.cpp \
           TestFPTree.cpp \
           TestFPGrowth.cpp
