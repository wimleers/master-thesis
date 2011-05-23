#ifndef TESTPATTERNTREE_H
#define TESTPATTERNTREE_H

#include <QtTest/QtTest>
#include "../PatternTree.h"

using namespace Analytics;

class TestPatternTree : public QObject {
    Q_OBJECT

private slots:
    void basic();
    void additionsRemainInSync();
};

#endif // TESTPATTERNTREE_H
