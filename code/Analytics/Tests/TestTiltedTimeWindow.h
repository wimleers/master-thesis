#ifndef TESTTILTEDTIMEWINDOW_H
#define TESTTILTEDTIMEWINDOW_H

#include <QtTest/QtTest>
#include "../TiltedTimeWindow.h"

using namespace Analytics;

class TestTiltedTimeWindow : public QObject {
    Q_OBJECT

private slots:
    void basic();
};

#endif // TESTTILTEDTIMEWINDOW_H
