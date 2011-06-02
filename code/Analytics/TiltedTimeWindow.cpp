#include "TiltedTimeWindow.h"

namespace Analytics {

    uint TiltedTimeWindow::GranularityBucketCount[TTW_NUM_GRANULARITIES]      = {  4, 24, 31, 12,  1 };
    uint TiltedTimeWindow::GranularityBucketOffset[TTW_NUM_GRANULARITIES]     = {  0,  4, 28, 59, 71 };
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

    void TiltedTimeWindow::appendQuarter(SupportCount supportCount, quint32 updateID) {
        this->lastUpdate = updateID;
        store(GRANULARITY_QUARTER, supportCount);
    }

    /**
     * Drop the tail. Only allow entire granularities to be dropped. Otherwise
     * tail dropping/pruning may result in out-of-sync TiltedTimeWindows. This
     * of course leads to TiltedTimeWindows tipping over to the higher-level
     * granularities at different points in time, which would cause incorrect
     * results.
     *
     * @param start
     *   The granularity starting from which all buckets should be dropped.
     */
    void TiltedTimeWindow::dropTail(Granularity start) {
        // Find the granularity to which it belongs and reset every
        // granularity along the way.
        Granularity g;
        for (g = (Granularity) (TTW_NUM_GRANULARITIES - 1); g >= start; g = (Granularity) ((int) g - 1))
            this->reset(g);
    }

    /**
     * Get the support in this TiltedTimeWindow for a range of buckets.
     *
     * @param from
     *   The range starts at this bucket.
     * @param to
     *   The range ends at this bucket.
     * @return
     *   The total support in the buckets in the given range.
     */
    SupportCount TiltedTimeWindow::getSupportForRange(uint from, uint to) const {
        Q_ASSERT(from <= to);
        Q_ASSERT(from < TTW_NUM_BUCKETS);
        Q_ASSERT(to   < TTW_NUM_BUCKETS);
        // "from" and "to" are unsigned integers, hence they're always >= 0.

        // Return 0 if this TiltedTimeWindow is empty.
        if (this->oldestBucketFilled == -1)
            return 0;

        // Otherwise, count the sum.
        SupportCount sum = 0;
        for (uint i = from; i <= to && i <= (uint) this->oldestBucketFilled; i++) {
            if (this->buckets[i] != (uint) TTW_BUCKET_UNUSED)
                sum += this->buckets[i];
        }

        return sum;
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
     * Reset a granularity.
     *
     * @param granularity
     *   The granularity that should be reset.
     */
    void TiltedTimeWindow::reset(Granularity granularity) {
        int offset = GranularityBucketOffset[granularity];
        int count = GranularityBucketCount[granularity];

        // Reset this granularity's buckets.
        memset(this->buckets + offset, TTW_BUCKET_UNUSED, count * sizeof(int));

        // Update this granularity's used capacity..
        this->capacityUsed[granularity] = 0;

        // Update oldestBucketFilled by resetting it to the beginning of this
        // granularity, but only if it is in fact currently pointing to a
        // position *within* this granularity (i.e. if it is in the range
        // [offset, offset + count - 1]).
        if (this->oldestBucketFilled > offset - 1 && this->oldestBucketFilled < offset + count)
            this->oldestBucketFilled = offset - 1;
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
        if (this->oldestBucketFilled < (int) (offset + this->capacityUsed[granularity] - 1))
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
