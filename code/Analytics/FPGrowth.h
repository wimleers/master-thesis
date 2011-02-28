#ifndef FPGROWTH_H
#define FPGROWTH_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <QString>
#include <QStringList>
#include <QRegExp>
#include <math.h>

#include "Item.h"
#include "Constraints.h"
#include "FPNode.h"
#include "FPTree.h"


namespace Analytics {

#ifdef DEBUG
//    #define FPGROWTH_DEBUG 1
#endif

    class FPGrowth : public QObject {
        Q_OBJECT

    public:
        FPGrowth(const QList<QStringList> & transactions, SupportCount minSupportAbsolute);
        ~FPGrowth();

        // Core functionality.
        void setItemConstraints(const QSet<ItemName> & constraints, ItemConstraintType type);
        QList<ItemList> mineFrequentItemsets();

        // Ability to calculate support for any itemset; necessary to
        // calculate confidence for candidate association rules.
        SupportCount calculateSupportCountUpperBound(const ItemList & itemset) const;
        SupportCount calculateSupportCountExactly(const ItemList & itemset) const;

        ItemID getItemID(ItemName name) const { return this->itemNameIDHash[name]; }
#ifdef DEBUG
        ItemIDNameHash * getItemIDNameHash() { return &this->itemIDNameHash; }
#endif

    protected slots:
        void processTransaction(const Transaction & transaction);

    protected:
        // Static methods.
        static QList<ItemID> sortItemIDsByDecreasingSupportCount(const QHash<ItemID, SupportCount> & itemSupportCounts);
        static QList<ItemList> filterPrefixPaths(const QList<ItemList> & prefixPaths, SupportCount minSupportAbsolute);

        // Methods.
        void scanTransactions();
        void buildFPTree();
        Transaction optimizeTransaction(const Transaction & transaction) const;
        QList<ItemList> generateFrequentItemsets(const FPTree * tree, const ItemList & suffix = ItemList());

        // Properties.
        FPTree * tree;
        Constraints constraints;
        QList<QStringList> transactions;

        SupportCount minSupportAbsolute;

        QHash<ItemID, SupportCount> totalFrequentSupportCounts;
        QList<ItemID> frequentItemIDsSortedByTotalSupportCount;

        ItemIDNameHash itemIDNameHash;
        ItemNameIDHash itemNameIDHash;
    };

}
#endif // FPGROWTH_H
