#ifndef TILTEDTIMEWINDOW_H
#define TILTEDTIMEWINDOW_H

#include <stdint.h>
#include <QVector>

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
        void dropTail(int start);

        // Operator overloads (must be defined in the header file).
        // These allow for elegant reuse of the FPNode template class.
        inline TiltedTimeWindow & operator+=(SupportCount support) {
            this->appendQuarter(support);
            return *this;
        }
        inline TiltedTimeWindow & operator=(SupportCount support) {
            this->appendQuarter(support);
            return *this;
        }

        // Unit testing helper method.
        const QVector<SupportCount> & getBuckets();

    protected:
        // Methods.
        void reset(Granularity granularity, int startBucket = 0);
        void shift(Granularity granularity);
        void store(Granularity granularity, SupportCount supportCount);

        // Properties.
        SupportCount buckets[TTW_NUM_BUCKETS];
        int bucketLastFilled[TTW_NUM_GRANULARITIES];
        int capacityUsed[TTW_NUM_GRANULARITIES];
        // TODO: support starting at another time of the day than 00:00
//        QDateTime startTime;

        static int GranularityBucketCount[TTW_NUM_GRANULARITIES];
        static int GranularityBucketOffset[TTW_NUM_GRANULARITIES];
    };
}

#endif // TILTEDTIMEWINDOW_H
