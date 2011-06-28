#ifndef ANALYTICS_H
#define ANALYTICS_H

#include <QObject>
#include <QTime>
#include <QList>
#include <QHash>
#include <QStringList>
#include <QPair>
#include <QStandardItemModel>
#include <QStandardItem>

#include <QThread>
#include <QWaitCondition>
#include <QMutex>

#include "Item.h"
#include "Constraints.h"
#include "FPGrowth.h"
#include "FPStream.h"
#include "RuleMiner.h"

typedef uint Time;

namespace Analytics {

    class Analyst : public QObject {
        Q_OBJECT

    public:
        Analyst(double minSupport, double maxSupportError, double minConfidence);
        ~Analyst();
        void addFrequentItemsetItemConstraint(ItemName item, ItemConstraintType type);
        void addRuleConsequentItemConstraint(ItemName item, ItemConstraintType type);

        // Override moveToThread to also move the FPStream instance.
        void moveToThread(QThread * thread);

        // UI integration.
        QStandardItemModel * getConceptHierarchyModel() const { return this->conceptHierarchyModel; }
        QPair<ItemName, ItemNameList> extractEpisodeFromItemset(ItemIDList itemset) const;

    signals:
        // Signals for UI.
        void analyzing(bool, Time start, Time end, int pageViews, int transactions);
        void analyzedDuration(int duration);
        void stats(Time start, Time end, int pageViews, int transactions, int uniqueItems, int frequentItems, int patternTreeSize);
        void mining(bool);
        void minedDuration(int duration);

        // Signals for calculations.
        void processedBatch();
        void minedRules(uint from, uint to, QList<Analytics::AssociationRule> associationRules, Analytics::SupportCount eventsInTimeRange);
        void comparedMinedRules(uint fromOlder, uint toOlder,
                                uint fromNewer, uint toNewer,
                                QList<Analytics::AssociationRule> intersectedRules,
                                QList<Analytics::AssociationRule> olderRules,
                                QList<Analytics::AssociationRule> newerRules,
                                QList<Analytics::AssociationRule> comparedRules,
                                QList<Analytics::Confidence> confidenceVariance,
                                QList<float> supportVariance,
                                Analytics::SupportCount eventsInIntersectedTimeRange,
                                Analytics::SupportCount eventsInOlderTimeRange,
                                Analytics::SupportCount eventsInNewerTimeRange);

    public slots:
        void analyzeTransactions(const QList<QStringList> & transactions, double transactionsPerEvent, Time start, Time end);
        void mineRules(uint from, uint to);
        void mineAndCompareRules(uint fromOlder, uint toOlder, uint fromNewer, uint toNewer);

    protected slots:
        void fpstreamProcessedBatch();

    protected:
        void performMining(const QList<QStringList> & transactions, double transactionsPerEvent);
        void updateConceptHierarchyModel(int itemsAlreadyProcessed);

        FPStream * fpstream;
        double minSupport;
        double maxSupportError;
        double minConfidence;

        Constraints frequentItemsetItemConstraints;
        Constraints ruleConsequentItemConstraints;

        ItemIDNameHash itemIDNameHash;
        ItemNameIDHash itemNameIDHash;
        ItemIDList sortedFrequentItemIDs;

        // Stats for the UI.
        int currentBatchStartTime;
        int currentBatchEndTime;
        int currentBatchNumPageViews;
        int currentBatchNumTransactions;
        int allBatchesStartTime;
        int allBatchesNumPageViews;
        int allBatchesNumTransactions;
        QTime timer;

        // Browsable concept hierarchy for the UI.
        int uniqueItemsBeforeMining;
        QStandardItemModel * conceptHierarchyModel;
        QHash<ItemName, QStandardItem *> conceptHierarchyHash;
    };
}

#endif // ANALYST_H
