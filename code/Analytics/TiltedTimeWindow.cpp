#include "TiltedTimeWindow.h"

namespace Analytics {

    int  TiltedTimeWindow::GranularityBucketCount[TTW_NUM_GRANULARITIES]      = {  4, 24, 31, 12,  1 };
    int  TiltedTimeWindow::GranularityBucketOffset[TTW_NUM_GRANULARITIES]     = {  0,  4, 28, 59, 71 };
    char TiltedTimeWindow::GranularityChar[TTW_NUM_GRANULARITIES]             = {'Q','H','D','M','Y' };


    //--------------------------------------------------------------------------
    // Public methods.

    TiltedTimeWindow::TiltedTimeWindow() {
        this->oldestBucketFilled = -1;
        this->lastUpdate = 0;
        for (int b = 0; b < TTW_NUM_BUCKETS; b++)
            this->buckets[b] = TTW_BUCKET_UNUSED;
        for (int g = 0; g < TTW_NUM_GRANULARITIES; g++)
            this->capacityUsed[g] = 0;
    }

    void TiltedTimeWindow::appendQuarter(SupportCount supportCount, uint32_t updateID) {
        this->lastUpdate = updateID;
        store(GRANULARITY_QUARTER, supportCount);
    }

    void TiltedTimeWindow::dropTail(int start) {
        // Find the granularity to which it belongs and reset every
        // granularity along the way.
        Granularity g;
        for (g = (Granularity) (TTW_NUM_GRANULARITIES - 1); g >= 0; g = (Granularity) ((int) g - 1)) {
            if (start >= this->GranularityBucketOffset[g])
                break;
            else
                this->reset(g);
        }

        // Now decide what to do with the granularity in which this tail
        // was started to be pruned. Reset the granularity starting at
        // the starting position of the tail pruning; this will
        // automatically reset the entire granularity when this is the
        // first bucket of the granularity.
        this->reset(g, start - this->GranularityBucketOffset[g]);
    }

    QVector<SupportCount> TiltedTimeWindow::getBuckets(int numBuckets) const {
        Q_ASSERT(numBuckets <= TTW_NUM_BUCKETS);

        QVector<SupportCount> v;
        for (int i = 0; i < numBuckets; i++)
            v.append(this->buckets[i]);
        return v;
    }


    //--------------------------------------------------------------------------
    // Private methods.

    /**
     * Reset a granularity. Defaults to resetting the entire granularity,
     * but allows for a partial reset.
     *
     * @param granularity
     *   The granularity that should be reset.
     * @param startBucket
     *   When performing a partial reset,
     */
    void TiltedTimeWindow::reset(Granularity granularity, int startBucket) {
        int offset = GranularityBucketOffset[granularity];
        int count = GranularityBucketCount[granularity];

        // Reset this granularity's buckets.
        memset(this->buckets + offset + startBucket, TTW_BUCKET_UNUSED, (count - startBucket ) * sizeof(int));

        // Update this granularity's used capacity..
        this->capacityUsed[granularity] = startBucket;

        // Update oldestBucketFilled.
        if (this->oldestBucketFilled > offset + startBucket - 1) {
            // Reset from first bucket in the first granularity: it's now
            // completely empty again.
            if (startBucket == 0 && granularity == GRANULARITY_QUARTER) {
                this->oldestBucketFilled = -1;
            }
            // When the start bucket is zero and we're not in the first
            // granularity, then we should look at the preceding granularity
            // to determine what the oldest filled bucket is.
            // After all, suppose we have the case {Q={1}, H={1}} and we are
            // resetting the second granularity (hour) with start position 0.
            // Without this special check, the oldest filled bucket would then
            // become bucket 3, whereas it really is bucket 0!
            else if (startBucket == 0 && granularity > 0) {
                Granularity oneLess = (Granularity) (granularity - 1);
                this->oldestBucketFilled = GranularityBucketOffset[oneLess] + this->capacityUsed[oneLess];
            }
            // Finally, the simple case where the start bucket is not zero
            // (meaning that we're not resetting the entire granularity. Then
            // we can simply move the oldestBucketFilled to just before the
            // start bucket.
            else if (startBucket > 0) {
                this->oldestBucketFilled = offset + startBucket - 1;
            }
        }
    }

    /**
     * Shift the support counts from one granularity to the next.
     *
     * @param granularity
     *   The granularity that should be shifted.
     */
    void TiltedTimeWindow::shift(Granularity granularity) {
        // If the next granularity does not exist, reset this granularity.
        if (granularity + 1 > TTW_NUM_GRANULARITIES - 1)
            this->reset(granularity);

        int offset = GranularityBucketOffset[granularity];
        int count  = GranularityBucketCount[granularity];

        // Calculate the sum of this granularity's buckets.
        SupportCount sum = 0;
        for (int bucket = 0; bucket < count; bucket++)
            sum += buckets[offset + bucket];

        // Reset this granularity.
        this->reset(granularity);

        // Store the sum of the buckets in the next granularity.
        this->store((Granularity) (granularity + 1), sum);
    }

    /**
     * Store a next support count in a granularity.
     *
     * @param granularity
     *   The granularity to which this support count should be appended.
     * @param supportCount
     *   The supportCount that should be appended.
     */
    void TiltedTimeWindow::store(Granularity granularity, SupportCount supportCount) {
        int offset       = GranularityBucketOffset[granularity];
        int count        = GranularityBucketCount[granularity];
        int capacityUsed = this->capacityUsed[granularity];

        // If the current granularity's maximum capacity has been reached,
        // then shift it to the next (less granular) granularity.
        if (capacityUsed == count) {
            this->shift(granularity);
            capacityUsed = this->capacityUsed[granularity];
        }

        // Store the value (in the first bucket of this granularity, which
        // means we'll have to move the data in previously filled in buckets
        // in this granularity) and update this granularity's capacity.
        if (capacityUsed > 0)
            memmove(buckets + offset + 1, buckets + offset, capacityUsed * sizeof(int));
        buckets[offset + 0] = supportCount;
        this->capacityUsed[granularity]++;

        // Update oldestbucketFilled.
        if (this->oldestBucketFilled < offset + this->capacityUsed[granularity] - 1)
            this->oldestBucketFilled = offset + this->capacityUsed[granularity] - 1;
    }

#ifdef DEBUG
    QDebug operator<<(QDebug dbg, const TiltedTimeWindow & ttw) {
        int capacityUsed, offset;
        QVector<SupportCount> buckets = ttw.getBuckets();

        dbg.nospace() << "{";

        Granularity g;
        for (g = (Granularity) 0; g < TTW_NUM_GRANULARITIES; g = (Granularity) ((int) g + 1)) {
            capacityUsed = ttw.getCapacityUsed(g);
            if (capacityUsed == 0)
                break;

            dbg.nospace() << TiltedTimeWindow::GranularityChar[g] << "={";

            // Print the contents of this granularity.
            offset = TiltedTimeWindow::GranularityBucketOffset[g];
            for (int b = 0; b < capacityUsed; b++) {
                if (b > 0)
                    dbg.nospace() << ", ";

                dbg.nospace() << buckets[offset + b];
            }

            dbg.nospace() << "}";
            if (g < TTW_NUM_GRANULARITIES - 1 && ttw.getCapacityUsed((Granularity) (g + 1)) > 0)
                dbg.nospace() << ", ";
        }

        dbg.nospace() << "} (lastUpdate=" << ttw.getLastUpdate() << ")";

        return dbg.nospace();
    }

#endif
}
