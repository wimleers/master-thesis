#ifndef FPSTREAM_H
#define FPSTREAM_H

#include <QObject>
#include <QList>
#include <QVector>
#include <QMutex>
#include <QMutexLocker>

#include "Item.h"
#include "Constraints.h"
#include "FPNode.h"
#include "FPTree.h"
#include "FPGrowth.h"
#include "PatternTree.h"

namespace Analytics {

#ifdef DEBUG
//    #define FPSTREAM_DEBUG 1
#endif

    class FPStream : public QObject {
        Q_OBJECT

    public:
        FPStream(double minSupport,
                 double maxSupportError,
                 ItemIDNameHash * itemIDNameHash,
                 ItemNameIDHash * itemNameIDHash,
                 ItemIDList * sortedFrequentItemIDs);
        SupportCount calculateMinSupportForRange(uint from, uint to) const;

        void setConstraints(const Constraints & constraints) { this->constraints = constraints; }
        void setConstraintsToPreprocess(const Constraints & constraints) { this->constraintsToPreprocess = constraints; }

        // Unit testing helper method.
        const PatternTree & getPatternTree() const { return this->patternTree; }

        // Static methods (public to allow for unit testing).
        static Granularity calculateDroppableTail(const TiltedTimeWindow & window,
                                                  double minSupport,
                                                  double maxSupportError,
                                                  const TiltedTimeWindow & eventsPerBatch);

    signals:
        void mineForFrequentItemsupersets(const FPTree * tree, const FrequentItemset & suffix);

    public slots:
        void processBatchTransactions(const QList<QStringList> & transactions, double transactionsPerEvent = 1.0);
        void processFrequentItemset(const FrequentItemset & frequentItemset,
                                    bool frequentItemsetMatchesConstraints,
                                    const FPTree * ctree);
        void branchCompleted(const ItemIDList & itemset);

    protected:
        // Methods.
        void updateUnaffectedNodes(FPNode<TiltedTimeWindow> * node);

        // Properties related to the entire state over time.
        PatternTree patternTree;
        TiltedTimeWindow transactionsPerBatch;
        TiltedTimeWindow eventsPerBatch;

        // Properties related to configuration.
        bool initialBatchProcessed;
        double minSupport;
        double maxSupportError;
        Constraints constraints;
        Constraints constraintsToPreprocess;

        // Properties that are updated in each batch.
        ItemIDNameHash * itemIDNameHash;
        ItemNameIDHash * itemNameIDHash;
        ItemIDList     * f_list; // sortedFrequentItemIDs would be a better
                                 // name, but it's called f_list in the
                                 // FP-Stream paper.

        // Properties relating to the current batch being processed.
        QMutex statusMutex;
        bool processingBatch;
        quint32 currentBatchID;
        FPGrowth * currentFPGrowth;
        QList<ItemIDList> supersetsBeingCalculated;
    };

}
#endif // FPSTREAM_H
