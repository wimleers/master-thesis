#include "TestTiltedTimeWindow.h"

void TestTiltedTimeWindow::basic() {
    TiltedTimeWindow * ttw = new TiltedTimeWindow();

    QList<SupportCount> supportCounts;
    // First hour: first four quarters.
    supportCounts << 45 << 67 << 88 << 93;
    // Second hour.
    supportCounts << 34 << 49 << 36 << 97;
    // Third hour.
    supportCounts << 50 << 50 << 50 << 50;
    // Hours 4-23.
    for (int i = 3; i <= 23; i++)
        supportCounts << 25 << 25 << 25 << 25;
    // First quarter of second day to provide tipping point: now the 24
    // hour buckets are all filled.
    supportCounts << 10;
    // Four more quarters, meaning that the first hour of the second day
    // will be completed *and* another quarter is added, which will provide
    // the tipping point to fill the first day bucket.
    supportCounts << 10 << 10 << 10 << 222;

    // First hour.
    for (int i = 0; i < 4; i++)
        ttw->appendQuarter(supportCounts[i]);
    QVector<SupportCount> buckets = ttw->getBuckets();
    buckets.resize(4);
    QCOMPARE(buckets, QVector<SupportCount>() << 93 << 88 << 67 << 45);

    // Second hour.
    for (int i = 4; i < 8; i++)
        ttw->appendQuarter(supportCounts[i]);
    buckets = ttw->getBuckets();
    buckets.resize(5);
    QCOMPARE(buckets, QVector<SupportCount>() <<  97 << 36 << 49 << 34
                                              << 293);

    // Third hour.
    for (int i = 8; i < 12; i++)
        ttw->appendQuarter(supportCounts[i]);
    buckets = ttw->getBuckets();
    buckets.resize(6);
    QCOMPARE(buckets, QVector<SupportCount>() <<  50 <<  50 <<  50 <<  50
                                              << 216 << 293);

    // Hours 4-23.
    for (int i = 12; i < 96 ; i++)
        ttw->appendQuarter(supportCounts[i]);
    buckets = ttw->getBuckets();
    buckets.resize(28);

    QCOMPARE(buckets, QVector<SupportCount>() <<  25 <<  25 <<  25 <<  25
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 200
                                              << 216 << 293 <<  -1);

    // First quarter of second day to provide tipping point: now the 24
    // hour buckets are all filled.
    ttw->appendQuarter(supportCounts[96]);
    buckets = ttw->getBuckets();
    buckets.resize(28);
    QCOMPARE(buckets, QVector<SupportCount>() <<  10 <<  -1 <<  -1 << -1
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 200 << 216 << 293);

    // Four more quarters, meaning that the first hour of the second day
    // will be completed *and* another quarter is added, which will provide
    // the tipping point to fill the first day bucket.
    for (int i = 97; i < 101 ; i++)
        ttw->appendQuarter(supportCounts[i]);
    buckets = ttw->getBuckets();
    buckets.resize(29);
    QCOMPARE(buckets, QVector<SupportCount>() << 222 <<  -1 <<  -1 << -1
                                              <<  40 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              << 2809); // 2809 = 21*100 + 200 + 216 + 293


    // Drop tail starting at bucket 1. This means only the value in the
    // first bucket (bucket 0) is kept, and all the rest is reset.
    ttw->dropTail(1);
    buckets = ttw->getBuckets();
    QCOMPARE(buckets[0], (SupportCount) 222);
    for (int i = 1; i < TTW_NUM_BUCKETS; i++)
        QCOMPARE(buckets[i], (SupportCount) -1);

    delete ttw;
}
