#ifndef TILTEDTIMEWINDOW_H
#define TILTEDTIMEWINDOW_H

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
    #define TTW_BUCKET_UNUSED -1


    class TiltedTimeWindow {
    public:
        TiltedTimeWindow();
        void appendQuarter(SupportCount s, quint32 updateID);
        bool isEmpty() const { return this->oldestBucketFilled == -1; }
        quint32 getLastUpdate() const { return this->lastUpdate; }
        void dropTail(Granularity start);
        int getOldestBucketFilled() const { return this->oldestBucketFilled; }
        uint getCapacityUsed(Granularity g) const { return this->capacityUsed[g]; }
        SupportCount getSupportForRange(uint from, uint to) const;

        // Unit testing helper method.
        QVector<SupportCount> getBuckets(int numBuckets = TTW_NUM_BUCKETS) const;

        // Properties.
        SupportCount buckets[TTW_NUM_BUCKETS];
        int oldestBucketFilled;

        // Static properties
        static uint GranularityBucketCount[TTW_NUM_GRANULARITIES];
        static uint GranularityBucketOffset[TTW_NUM_GRANULARITIES];
        static char GranularityChar[TTW_NUM_GRANULARITIES];

    protected:
        // Methods.
        void reset(Granularity granularity);
        void shift(Granularity granularity);
        void store(Granularity granularity, SupportCount supportCount);

        // Properties.
        uint capacityUsed[TTW_NUM_GRANULARITIES];
        quint32 lastUpdate;
    };

#ifdef DEBUG
    QDebug operator<<(QDebug dbg, const TiltedTimeWindow & ttw);
#endif
}

#endif // TILTEDTIMEWINDOW_H
