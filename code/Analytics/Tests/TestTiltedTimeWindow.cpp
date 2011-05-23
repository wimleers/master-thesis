#include "TestTiltedTimeWindow.h"


/**
 * IMPORTANT NOTE:
 * Each time when the SupportCount -1 is being used in this test, it's meant
 * to be TTW_BUCKET_UNUSED. However, for legibility purposes, I've opted to
 * directly write -1 instead.
 */
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
    supportCounts << 10 << 10 << 10 << 20;
    // And finally, four more quarters, which will ensure there are 2 hours
    // of the second day.
    supportCounts << 20 << 20 << 20 << 30;

    // First hour.
    for (int i = 0; i < 4; i++)
        ttw->appendQuarter(supportCounts[i], i);
    QCOMPARE(ttw->getBuckets(4), QVector<SupportCount>() << 93 << 88 << 67 << 45);
    QCOMPARE(ttw->oldestBucketFilled, 3);
    QCOMPARE(ttw->getLastUpdate(), (unsigned int) 3);

    // Second hour.
    for (int i = 4; i < 8; i++)
        ttw->appendQuarter(supportCounts[i], i);
    QCOMPARE(ttw->getBuckets(5), QVector<SupportCount>() <<  97 << 36 << 49 << 34
                                              << 293);
    QCOMPARE(ttw->oldestBucketFilled, 4);
    QCOMPARE(ttw->getLastUpdate(), (unsigned int) 7);

    // Third hour.
    for (int i = 8; i < 12; i++)
        ttw->appendQuarter(supportCounts[i], i);
    QCOMPARE(ttw->getBuckets(6), QVector<SupportCount>() <<  50 <<  50 <<  50 <<  50
                                              << 216 << 293);
    QCOMPARE(ttw->oldestBucketFilled, 5);
    QCOMPARE(ttw->getLastUpdate(), (unsigned int) 11);

    // Hours 4-23.
    for (int i = 12; i < 96 ; i++)
        ttw->appendQuarter(supportCounts[i], i);
    QCOMPARE(ttw->getBuckets(28), QVector<SupportCount>() <<  25 <<  25 <<  25 <<  25
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 200
                                              << 216 << 293 <<  -1);
    QCOMPARE(ttw->oldestBucketFilled, 26);
    QCOMPARE(ttw->getLastUpdate(), (unsigned int) 95);

    // First quarter of second day to provide tipping point: now the 24
    // hour buckets are all filled.
    ttw->appendQuarter(supportCounts[96], 96);
    QCOMPARE(ttw->getBuckets(28), QVector<SupportCount>() <<  10 <<  -1 <<  -1 << -1
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 100 << 100 << 100
                                              << 200 << 216 << 293);
    QCOMPARE(ttw->oldestBucketFilled, 27);
    QCOMPARE(ttw->getLastUpdate(), (unsigned int) 96);

    // Four more quarters, meaning that the first hour of the second day
    // will be completed *and* another quarter is added, which will provide
    // the tipping point to fill the first day bucket.
    for (int i = 97; i < 101 ; i++)
        ttw->appendQuarter(supportCounts[i], i);
    QCOMPARE(ttw->getBuckets(29), QVector<SupportCount>() << 20 <<  -1 <<  -1 << -1
                                              <<  40 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              <<  -1 <<  -1 <<  -1
                                              << 2809); // 2809 = 21*100 + 200 + 216 + 293
    QCOMPARE(ttw->oldestBucketFilled, 28);
    QCOMPARE(ttw->getLastUpdate(), (unsigned int) 100);

    // Four more quarters, meaning that the second hour of the second day will
    // be completed. This is a test to check if the "oldestBucketFilled"
    // variable updates correctly: it should remain set to 28, and should not
    // be reset to 5. Since the second hour is added (which means the first
    // hour shifts from bucket 4 to bucket 5), this is a logic edge case that
    // may be expected.
    for (int i = 101; i < 105; i++)
        ttw->appendQuarter(supportCounts[i], i);
    QCOMPARE(ttw->oldestBucketFilled, 28);
    QCOMPARE(ttw->getLastUpdate(), (unsigned int) 104);

    // Drop tail starting at Granularity 1. This means only the value in the
    // first granularity (buckets 0, 1, 2 and 3) are kept, and all subsequent
    // granularities (and buckets) are reset.
    ttw->dropTail((Granularity) 1);
    QVector<SupportCount> buckets = ttw->getBuckets();
    QCOMPARE(buckets[0], (SupportCount) 30);
    for (int i = 1; i < TTW_NUM_BUCKETS; i++)
        QCOMPARE(buckets[i], (SupportCount) -1);
    QCOMPARE(ttw->oldestBucketFilled, 3);
    QCOMPARE(ttw->getLastUpdate(), (unsigned int) 104);

    delete ttw;
}
