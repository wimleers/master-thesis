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
        FPGrowth(const QList<QStringList> & transactions, float minSupportRelative);
        ~FPGrowth();

        void setTransactionRequirements(QList<ItemName> items);
        QList<ItemList> mineFrequentItemsets();

        ItemID getItemID(ItemName name) const { return this->itemNameIDHash[name]; }
#ifdef DEBUG
        ItemIDNameHash * getItemIDNameHash() { return &this->itemIDNameHash; }
#endif

    protected slots:
        void processTransaction(Transaction transaction);

    protected:
        // Static methods.
        static QList<ItemID> sortItemIDsByDecreasingSupportCount(QHash<ItemID, SupportCount> itemSupportCounts);

        // Methods.
        void scanTransactions();
        void buildFPTree();
        Transaction optimizeTransaction(Transaction transaction) const;
        QList<ItemList> generateFrequentItemsets(FPTree * tree, ItemList suffix = ItemList());

        // Properties.
        FPTree * tree;
        QList<QStringList> transactions;

        float minSupportRelative;
        SupportCount minSupportAbsolute;

        QHash<ItemID, SupportCount> totalFrequentSupportCounts;
        QList<ItemID> frequentItemIDsSortedByTotalSupportCount;

        ItemIDNameHash itemIDNameHash;
        ItemNameIDHash itemNameIDHash;

        QList<ItemName> transactionRequirements;
    };

}
#endif // FPGROWTH_H
