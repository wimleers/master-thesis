#ifndef FPGROWTH_H
#define FPGROWTH_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>
#include <math.h>

#include "Item.h"
#include "FPNode.h"
#include "FPTree.h"


namespace Analytics {

#ifdef DEBUG
//    #define FPGROWTH_DEBUG 1
#endif

    class FPGrowth : public QObject {
        Q_OBJECT

    public:
        FPGrowth(const QList<QStringList> & transactions, float minimumSupport);
        ~FPGrowth();
        void setFilterItems(QList<ItemName> items);
        ItemID getItemID(ItemName name) const { return this->itemNameIDHash[name]; }
        void preprocessingPhase1();
        void preprocessingPhase2();
        QList<ItemList> calculatingPhase1();

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
        QList<ItemName> filterItems;

        Transaction optimizeTransaction(Transaction transaction) const;
        void calculateItemsSortedBySupportCount();
        ItemIDList determineSuffixOrder() const;

        QList<ItemList> generateFrequentItemsets(FPTree * tree, ItemList suffix = ItemList());
    };

}
#endif // FPGROWTH_H
