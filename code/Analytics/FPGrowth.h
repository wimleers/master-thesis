#ifndef FPGROWTH_H
#define FPGROWTH_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>
#include <math.h>
#include "typedefs.h"
#include "FPNode.h"
#include "FPTree.h"

#ifdef DEBUG
//#define FPGROWTH_DEBUG 0
#endif

class FPGrowth : public QObject {
    Q_OBJECT

public:
    FPGrowth(const QList<QStringList> & transactions, float minimumSupport);
    ~FPGrowth();
    ItemIDNameHash getItemIDNameHash() const { return this->itemIDNameHash; }
    void preprocessingPhase1();
    void preprocessingPhase2();
    QList<ItemList> calculatingPhase1();
    QList<SupportCount> calculatingPhase2(QList<ItemList> frequentItemsets);

    // Static (class) methods.
    static QList<SupportCount> calculateSupportCountsForFrequentItemsets(QList<ItemList> frequentItemsets);
    static SupportCount calculateSupportCountForFrequentItemset(ItemList frequentItemset);

#ifdef DEBUG
    ItemIDNameHash * getItemIDNameHash() { return &this->itemIDNameHash; }
#endif

protected slots:
    void processTransaction(Transaction transaction);

protected:
    ItemCountHash totalSupportCounts;
    ItemIDList itemsSortedByTotalSupportCount;
    FPTree * tree;
    QList<QStringList> transactions;
    float minimumSupport;
    SupportCount minimumSupportAbsolute;
    int numberTransactions;
    ItemIDNameHash itemIDNameHash;
    ItemNameIDHash itemNameIDHash;

    Transaction optimizeTransaction(Transaction transaction) const;
    void calculateItemsSortedBySupportCount();
    ItemIDList determineSuffixOrder() const;

    QList<ItemList> generateFrequentItemsets(FPTree* tree, ItemList suffix = ItemList());
};

#endif // FPGROWTH_H
