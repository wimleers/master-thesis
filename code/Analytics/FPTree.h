#ifndef FPTREE_H
#define FPTREE_H

#include <QHash>
#include <QPair>
#include <QDebug>
#include <QMetaType>
#include <QString>

#include "Item.h"
#include "FPNode.h"


namespace Analytics {
    class FPTree {
    public:
        FPTree();
        ~FPTree();

        // Accessors.
        FPNode<SupportCount> * getRoot() const { return this->root; }
        bool hasItemPath(ItemID itemID) const;
        ItemIDList getItemIDs() const { return this->itemPaths.keys(); }
        QList<FPNode<SupportCount> *> getItemPath(ItemID itemID) const;
        bool itemPathContains(ItemID itemID, FPNode<SupportCount> * node) const;
        SupportCount getItemSupport(ItemID item) const;
        QList<ItemList> calculatePrefixPaths(ItemID itemID) const;

        // Modifiers.
        void addTransaction(const Transaction & transaction);
        void buildTreeFromPrefixPaths(const QList<ItemList> & prefixPaths);

        // Static (class) methods.
        static QHash<ItemID, SupportCount> calculateSupportCountsForPrefixPaths(const QList<ItemList> & prefixPaths);

#ifdef DEBUG
        ItemIDNameHash * itemIDNameHash;
#endif

    protected:
        FPNode<SupportCount> * root;
        QHash<ItemID, QList<FPNode<SupportCount> *> > itemPaths;

        void init();
        void addNodeToItemPath(FPNode<SupportCount> * node);
    };

#ifdef DEBUG
    QDebug operator<<(QDebug dbg, const FPTree & tree);
    QString dumpHelper(const FPNode<SupportCount> & node, QString prefix = "");

    // QDebug output operators for SupportCount.
    QDebug operator<<(QDebug dbg, const FPNode<SupportCount> & node);
    QDebug operator<<(QDebug dbg, const QList<FPNode<SupportCount> *> & itemPath);
#endif
}

Q_DECLARE_METATYPE(Analytics::FPTree);

#endif // FPTREE_H
