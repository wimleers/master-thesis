#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include "stdint.h"
#include <QDebug>
#include <QHash>
#include <QList>
#include <QString>

// Generic data mining types.
typedef uint32_t ItemID; // Supports 2^32 *different* items. Change to uint64_t to support more.
#define ROOT_ITEMID 4294967295 // Largest supported value for uint32_t.
typedef QString ItemName;
typedef uint32_t SupportCount; // Supports 2^32 *total* items. Change to uint64_t to support more.
#define MAX_SUPPORT 4294967295
typedef QHash<ItemID, ItemName> ItemIDNameHash;
typedef QHash<ItemName, ItemID> ItemNameIDHash;
struct Item {
    Item() {}
    Item(ItemID id) : id(id), supportCount(1) {}
    Item(ItemID id, SupportCount supportCount) : id(id), supportCount(supportCount) {}

    ItemID id;
    /**
     * One would not expect SupportCount to be associated with an item.
     * However, this allows for cleaner code when building conditional
     * FP-trees. More specifically: the prefix paths that
     * FPTree::calculatePrefixPaths() returns already include the correct
     * SupportCount values (i.e. the number of times that itemset was included
     * in all transactions) and can be passed to FPTree::addTransaction()
     * *directly*. Otherwise, we'd have to repeatedly insert the prefix path,
     * to match the number of times that itemset was included in all
     * transactions
     * Each item occurs once in each transaction. Therefor, this defaults to
     * 1.
     */
    SupportCount supportCount;

#ifdef DEBUG
    Item(ItemID id, ItemIDNameHash * IDNameHash) : id(id), supportCount(1), IDNameHash(IDNameHash) {}
    Item(ItemID id, SupportCount supportCount, ItemIDNameHash * IDNameHash) : id(id), supportCount(supportCount), IDNameHash(IDNameHash) {}
    ItemIDNameHash * IDNameHash;
#endif
};
inline bool operator==(const Item & i1, const Item & i2) {
    return i1.id == i2.id && i1.supportCount == i2.supportCount;
}


// Generic data mining container types.
typedef QHash<ItemID, SupportCount> ItemCountHash;
typedef QList<ItemID> ItemIDList;
typedef QList<Item> ItemList;
typedef QList<SupportCount> ItemCountList;
typedef QList<Item> Transaction;
struct AssociationRuleStruct { ItemList antecedent; ItemList consequent; float confidence; };
typedef AssociationRuleStruct AssociationRule;



// FP-growth specific types.
class FPNode; // Work around cyclical dependency.

// FP-growth specific container types.
typedef QHash<ItemID, FPNode *> ItemIDFPNodeHash;
typedef QList<FPNode *> FPNodeList;


#ifdef DEBUG
// QDebug() streaming output operators.
QDebug operator<<(QDebug dbg, const Item & item);
QDebug operator<<(QDebug dbg, const Transaction & transaction);
QDebug operator<<(QDebug dbg, const AssociationRule & associationRule);
#endif

#endif // TYPEDEFS_H
