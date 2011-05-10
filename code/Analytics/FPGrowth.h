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
        FPGrowth(const QList<QStringList> & transactions, SupportCount minSupportAbsolute, ItemIDNameHash * itemIDNameHash, ItemNameIDHash * itemNameIDHash);
        ~FPGrowth();

        void setConstraints(const Constraints & constraints) { this->constraints = constraints; }
        void setConstraintsToPreprocess(const Constraints & constraints) { this->constraintsToPreprocess = constraints; }
        const Constraints & getPreprocessedConstraints() const { return this->constraintsToPreprocess; }

        QList<FrequentItemset> mineFrequentItemsets();

        // Ability to calculate support for any itemset; necessary to
        // calculate confidence for candidate association rules.
        SupportCount calculateSupportCountUpperBound(const ItemIDList & itemset) const;
        SupportCount calculateSupportCountExactly(const ItemIDList & itemset) const;

        ItemID getItemID(ItemName name) const { return this->itemNameIDHash->value(name); }
#ifdef DEBUG
        ItemIDNameHash * getItemIDNameHash() { return this->itemIDNameHash; }
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
        ItemIDList optimizeItemset(const ItemIDList & itemset) const;
        QList<FrequentItemset> generateFrequentItemsets(const FPTree * tree, const FrequentItemset & suffix = FrequentItemset());

        // Properties.
        FPTree * tree;
        Constraints constraints;
        Constraints constraintsToPreprocess;
        ItemIDNameHash * itemIDNameHash;
        ItemNameIDHash * itemNameIDHash;

        QList<QStringList> transactions;

        SupportCount minSupportAbsolute;

        QHash<ItemID, SupportCount> totalFrequentSupportCounts;
        QList<ItemID> frequentItemIDsSortedByTotalSupportCount;
    };

}
#endif // FPGROWTH_H
