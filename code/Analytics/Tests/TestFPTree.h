#ifndef TESTFPTREE_H
#define TESTFPTREE_H

#include <QtTest/QtTest>
#include <QFile>
#include "../FPTree.h"

using namespace Analytics;

class TestFPTree: public QObject {
    Q_OBJECT

private slots:
    void basic();
};

#endif // TESTFPTREE_H
