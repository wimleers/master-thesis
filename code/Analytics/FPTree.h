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
        FPTree(const QList<ItemList> & prefixPaths);
        ~FPTree();

        // Accessors.
        FPNode<SupportCount> * getRoot() const { return this->root; }
        bool hasItemPath(ItemID itemID) const;
        QList<ItemID> getItemIDs() const { return this->itemPaths.keys(); }
        QList<FPNode<SupportCount> *> getItemPath(ItemID itemID) const;
        bool itemPathContains(ItemID itemID, FPNode<SupportCount> * node) const;
        SupportCount getItemSupport(ItemID item) const;
        QList<ItemList> calculatePrefixPaths(ItemID itemID) const;

        // Modifiers.
        void addTransaction(const Transaction & transaction);

        // Static (class) methods.
        static QHash<ItemID, SupportCount> calculateSupportCountsForPrefixPaths(const QList<ItemList> & prefixPaths);

    protected:
        FPNode<SupportCount> * root;
        QHash<ItemID, QList<FPNode<SupportCount> *> > itemPaths;

        void init();
        void addNodeToItemPath(FPNode<SupportCount> * node);
    };

#ifdef DEBUG
    QDebug operator<<(QDebug dbg, const FPTree & tree);
    QString dumpHelper(const FPNode<SupportCount> & node, QString prefix = "");
#endif

}

Q_DECLARE_METATYPE(Analytics::FPTree);

#endif // FPTREE_H
