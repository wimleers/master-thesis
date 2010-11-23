#ifndef TESTPARSER_H
#define TESTPARSER_H

#include <QtTest/QtTest>
#include <QFile>
#include "../Parser.h"

using namespace EpisodesParser;

class TestParser: public QObject {
    Q_OBJECT

private slots:
//    void initTestCase() {}
//    void cleanupTestCase() {}
    void init();
    void cleanup();
    void parse();
    void mapLineToEpisodesLogLine_data();
    void mapLineToEpisodesLogLine();
};

#endif // TESTPARSER_H
