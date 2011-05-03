#ifndef TILTEDTIMEWINDOW_H
#define TILTEDTIMEWINDOW_H

#include <stdint.h>
#include <QVector>
#include <QDebug>

#include "Item.h"


namespace Analytics {

    enum Granularity {
      GRANULARITY_QUARTER,
      GRANULARITY_HOUR,
      GRANULARITY_DAY,
      GRANULARITY_MONTH,
      GRANULARITY_YEAR
    };

    #define TTW_NUM_GRANULARITIES 5
    #define TTW_NUM_BUCKETS 72


    class TiltedTimeWindow {
    public:
        TiltedTimeWindow();
        void appendQuarter(SupportCount s);

        const QVector<SupportCount> & getBuckets() { return this->buckets; }

    protected:
        // Methods.
        void reset(Granularity granularity);
        void shift(Granularity granularity);
        void store(Granularity granularity, SupportCount supportCount);

        // Properties.
        QVector<SupportCount> buckets;
        QVector<int> bucketLastFilled;
        // TODO: support starting at another time of the day than 00:00
//        QDateTime startTime;

        static int GranularityBucketCount[TTW_NUM_GRANULARITIES];
        static int GranularityBucketOffset[TTW_NUM_GRANULARITIES];
    };
}

#endif // TILTEDTIMEWINDOW_H
