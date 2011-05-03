#include "TiltedTimeWindow.h"

namespace Analytics {

    int TiltedTimeWindow::GranularityBucketCount[TTW_NUM_GRANULARITIES]      = {  4,  24, 31, 12,  1 };
    int TiltedTimeWindow::GranularityBucketOffset[TTW_NUM_GRANULARITIES]     = {  0,   4, 28, 59, 71 };


    //--------------------------------------------------------------------------
    // Public methods.

    TiltedTimeWindow::TiltedTimeWindow() {
        this->buckets          = QVector<SupportCount>(TTW_NUM_BUCKETS, 0);
        this->bucketLastFilled = QVector<int>(TTW_NUM_GRANULARITIES, -1);
    }

    void TiltedTimeWindow::appendQuarter(SupportCount supportCount) {
        store(GRANULARITY_QUARTER, supportCount);
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
        int count  = GranularityBucketCount[granularity];

        // Reset this granularity's buckets.
        for (int bucket = 0; bucket < count; bucket++)
            buckets[offset + bucket] = 0;

        // Update this granularity's last filled bucket.
        this->bucketLastFilled[granularity] = -1;
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
        int offset     = GranularityBucketOffset[granularity];
        int count      = GranularityBucketCount[granularity];

        // If the current granularity is full, then shift it to the next
        // granularity in line.
        if (this->bucketLastFilled[granularity] == count - 1)
            this->shift(granularity);

        // Store the value and update this granularity's last filled bucket.
        int nextBucket = this->bucketLastFilled[granularity] + 1;
        buckets[offset + nextBucket] = supportCount;
        this->bucketLastFilled[granularity] = nextBucket;
    }
}
