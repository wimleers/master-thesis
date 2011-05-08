#ifndef PATTERNTREE_H
#define PATTERNTREE_H

#include <stdint.h>
#include <QDebug>
#include <QMetaType>

#include "Item.h"
#include "TiltedTimeWindow.h"
#include "FPNode.h"


namespace Analytics {
    class PatternTree {
    public:
        PatternTree();
        ~PatternTree();

        // Accessors.
        FPNode<TiltedTimeWindow> * getRoot() const { return this->root; }
        TiltedTimeWindow * getPatternSupport(const ItemIDList & pattern) const;

        // Modifiers.
        void addPattern(const FrequentItemset & pattern, uint32_t updateID);

        // Static (class) methods.
        static ItemIDList getPatternForNode(FPNode<TiltedTimeWindow> const * const node);

    protected:
        FPNode<TiltedTimeWindow> * root;
    };

#ifdef DEBUG
    QDebug operator<<(QDebug dbg, const PatternTree & tree);
    QString dumpHelper(const FPNode<TiltedTimeWindow> & node, QString prefix = "");

    // QDebug output operators for FPNode<TiltedTimeWindow>.
    QDebug operator<<(QDebug dbg, const FPNode<TiltedTimeWindow> & node);

#endif

}

Q_DECLARE_METATYPE(Analytics::PatternTree);

#endif // PATTERNTREE_H
