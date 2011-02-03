#ifndef TESTRULEMINER_H
#define TESTRULEMINER_H

#include <QtTest/QtTest>
#include <QFile>
#include "../FPGrowth.h"
#include "../Ruleminer.h"

using namespace Analytics;

class TestRuleMiner : public QObject {
    Q_OBJECT

private slots:
//    void initTestCase() {}
//    void cleanupTestCase() {}
//    void init();
//    void cleanup();
    void basic();
};

#endif // TESTRULEMINER_H
