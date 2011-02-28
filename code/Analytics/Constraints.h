#ifndef CONSTRAINTS_H
#define CONSTRAINTS_H

#include <QRegExp>
#include <QList>
#include <QSet>

#include "Item.h"


namespace Analytics {

    enum ItemConstraintType {
        CONSTRAINT_POSITIVE_MATCH_ALL,
        CONSTRAINT_POSITIVE_MATCH_ANY,
        CONSTRAINT_NEGATIVE_MATCH_ALL,
        CONSTRAINT_NEGATIVE_MATCH_ANY
    };

    class Constraints {
    public:
        Constraints();

        bool empty() const { return this->itemConstraints.empty(); }

        void addItemConstraint(ItemName item, ItemConstraintType type);
        void setItemConstraints(const QSet<ItemName> & constraints, ItemConstraintType type);

        void preprocessItem(const ItemName & name, ItemID id);
        void removeItem(ItemID id);

        bool matchItemset(const ItemList & itemset) const;
        bool matchSearchSpace(const ItemList & frequentItemset, const QHash<ItemID, SupportCount> & prefixPathsSupportCounts) const;

    protected:
        static bool matchItemsetHelper(const ItemList & itemset, ItemConstraintType type, const QSet<ItemID> & constraintItems);
        static bool matchSearchSpaceHelper(const ItemList & frequentItemset, const QHash<ItemID, SupportCount> & prefixPathsSupportCounts, ItemConstraintType type, const QSet<ItemID> & constraintItems);

        void addPreprocessedItemConstraint(ItemConstraintType type, const ItemName & category, ItemID id);

        QHash<ItemConstraintType, QSet<ItemName> > itemConstraints;
        QHash<ItemConstraintType, QHash<ItemName, QSet<ItemID> > > preprocessedItemConstraints;
    };
}

#endif // CONSTRAINTS_H
