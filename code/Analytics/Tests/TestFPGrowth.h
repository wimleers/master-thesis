#ifndef TESTFPGROWTH_H
#define TESTFPGROWTH_H

#include <QtTest/QtTest>
#include <QFile>
#include "../FPGrowth.h"

using namespace Analytics;

class TestFPGrowth : public QObject {
    Q_OBJECT

private slots:
//    void initTestCase() {}
//    void cleanupTestCase() {}
//    void init();
//    void cleanup();
    void basic();
    void withFilter();
};

#endif // TESTFPGROWTH_H