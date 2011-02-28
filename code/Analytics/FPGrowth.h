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
#include "FPNode.h"
#include "FPTree.h"


namespace Analytics {

#ifdef DEBUG
//    #define FPGROWTH_DEBUG 1
#endif

    enum ItemConstraintType {
        CONSTRAINT_POSITIVE_MATCH_ALL,
        CONSTRAINT_POSITIVE_MATCH_ANY,
        CONSTRAINT_NEGATIVE_MATCH_ALL,
        CONSTRAINT_NEGATIVE_MATCH_ANY
    };

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

        // Constraint methods.
        bool constraintMatcher(const ItemList & itemset);
        bool searchSpaceConstraintMatcher(const ItemList & frequentItemset, const QHash<ItemID, SupportCount> & prefixPathsSupportCounts);
        static bool constraintMatchHelper(const ItemList & itemset, ItemConstraintType type, const QSet<ItemID> & constraintItems);
        static bool searchSpaceConstraintMatchHelper(const ItemList & frequentItemset, const QHash<ItemID, SupportCount> & prefixPathsSupportCounts, ItemConstraintType type, const QSet<ItemID> & constraintItems);

        // Methods.
        void scanTransactions();
        void buildFPTree();
        void preprocessItemForConstraints(const ItemName & name, ItemID id);
        void removeItemFromConstraints(ItemID id);
        void addPreprocessedItemConstraint(ItemConstraintType type, const ItemName & category, ItemID id);
        Transaction optimizeTransaction(const Transaction & transaction) const;
        QList<ItemList> generateFrequentItemsets(const FPTree * tree, const ItemList & suffix = ItemList());

        // Properties.
        FPTree * tree;
        QList<QStringList> transactions;

        SupportCount minSupportAbsolute;

        QHash<ItemID, SupportCount> totalFrequentSupportCounts;
        QList<ItemID> frequentItemIDsSortedByTotalSupportCount;

        ItemIDNameHash itemIDNameHash;
        ItemNameIDHash itemNameIDHash;

        QHash<ItemConstraintType, QSet<ItemName> > itemConstraints;
        QHash<ItemConstraintType, QHash<ItemName, QSet<ItemID> > > preprocessedItemConstraints;
    };

}
#endif // FPGROWTH_H
