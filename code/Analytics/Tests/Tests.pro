DEPENDPATH += ..
include (../Analytics.pri)

CONFIG += qtestlib
macx {
  CONFIG -= app_bundle
}
TARGET = Tests


HEADERS += TestFPTree.h \
           TestFPGrowth.h \
           TestRuleMiner.h \
           TestTiltedTimeWindow.h \
           TestPatternTree.h \
           TestFPStream.h
SOURCES += Tests.cpp \
           TestFPTree.cpp \
           TestFPGrowth.cpp \
           TestRuleMiner.cpp \
           TestTiltedTimeWindow.cpp \
           TestPatternTree.cpp \
           TestFPStream.cpp
