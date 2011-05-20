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

#define FPGROWTH_ASYNC true
#define FPGROWTH_SYNC false

    class FPGrowth : public QObject {
        Q_OBJECT

    public:
        FPGrowth(const QList<QStringList> & transactions, SupportCount minSupportAbsolute, ItemIDNameHash * itemIDNameHash, ItemNameIDHash * itemNameIDHash, ItemIDList * sortedFrequentItemIDs);
        ~FPGrowth();

        void setConstraints(const Constraints & constraints) { this->constraints = constraints; }
        void setConstraintsToPreprocess(const Constraints & constraints) { this->constraintsToPreprocess = constraints; }
        const Constraints & getPreprocessedConstraints() const { return this->constraintsToPreprocess; }

        QList<FrequentItemset> mineFrequentItemsets(bool asynchronous = true);

        // Ability to calculate support for any itemset; necessary to
        // calculate confidence for candidate association rules.
        SupportCount calculateSupportCountUpperBound(const ItemIDList & itemset) const;
        SupportCount calculateSupportCountExactly(const ItemIDList & itemset) const;

        ItemID getItemID(ItemName name) const { return this->itemNameIDHash->value(name); }
#ifdef DEBUG
        ItemIDNameHash * getItemIDNameHash() { return this->itemIDNameHash; }
#endif

    signals:
        void minedFrequentItemset(const FrequentItemset & frequentItemset, bool frequentItemsetMatchesConstraints, const FPTree * ctree);
        void branchCompleted(const ItemIDList & itemset);

    public slots:
        QList<FrequentItemset> generateFrequentItemsets(const FPTree * tree, const FrequentItemset & suffix, bool asynchronous = FPGROWTH_ASYNC);

    protected slots:
        void processTransaction(const Transaction & transaction);

    protected:
        // Static methods.
        static ItemIDList sortItemIDsByDecreasingSupportCount(const QHash<ItemID, SupportCount> & itemSupportCounts, const ItemIDList * const ignoreList);
        static QList<ItemList> filterPrefixPaths(const QList<ItemList> & prefixPaths, SupportCount minSupportAbsolute);

        // Methods.
        void scanTransactions();
        void buildFPTree();
        FPTree * considerFrequentItemsupersets(const FPTree * ctree, const ItemIDList & frequentItemset);
        Transaction optimizeTransaction(const Transaction & transaction) const;
        ItemIDList optimizeItemset(const ItemIDList & itemset) const;

        // Properties.
        FPTree * tree;
        Constraints constraints;
        Constraints constraintsToPreprocess;
        ItemIDNameHash * itemIDNameHash;
        ItemNameIDHash * itemNameIDHash;
        ItemIDList     * sortedFrequentItemIDs;

        QList<QStringList> transactions;

        SupportCount minSupportAbsolute;

        QHash<ItemID, SupportCount> totalFrequentSupportCounts;
    };

}
#endif // FPGROWTH_H
