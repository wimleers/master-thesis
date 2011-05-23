#include "Item.h"

namespace Analytics {

#ifdef DEBUG
    QDebug operator<<(QDebug dbg, const Item & i) {
        dbg.nospace() << i.IDNameHash->value(i.id).toStdString().c_str()
                << "("
                << i.id
                << ")="
                << i.supportCount;

        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const ItemIDList & pattern) {
        dbg.nospace() << "{";

        for (int i = 0; i < pattern.size(); i++) {
            if (i > 0)
                dbg.nospace() << ", ";

            dbg.nospace() << pattern[i];
        }
        dbg.nospace() << "}";

        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const Transaction & transaction) {
        QString itemOutput;

        dbg.nospace() << "[size=" << transaction.size() << "] {";

        for (int i = 0; i < transaction.size(); i++) {
            if (i > 0)
                dbg.nospace() << ", ";

            // Generate output for item.
            itemOutput.clear();
            QDebug(&itemOutput) << transaction[i];

            dbg.nospace() << itemOutput.toStdString().c_str();
        }
        dbg.nospace() << "}";

        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const FrequentItemset & frequentItemset) {
        QString itemOutput;

        dbg.nospace() << "({";
        itemIDHelper(dbg, frequentItemset.itemset, frequentItemset.IDNameHash);
        dbg.nospace() << "}, sup: "
                      << frequentItemset.support
                      << ")";

        return dbg.nospace();
    }

    QDebug operator<<(QDebug dbg, const AssociationRule & associationRule) {
        dbg.nospace() << "{";
        itemIDHelper(dbg, associationRule.antecedent, associationRule.IDNameHash);
        dbg.nospace() << "} => {";
        itemIDHelper(dbg, associationRule.consequent, associationRule.IDNameHash);
        dbg.nospace() << "}";

        dbg.nospace() << " (conf=" << associationRule.confidence << ")";

        return dbg.nospace();
    }

    QDebug itemIDHelper(QDebug dbg, const ItemIDList & itemset, ItemIDNameHash const * const IDNameHash) {
        for (int i = 0; i < itemset.size(); i++) {
            if (i > 0)
                dbg.nospace() << ", ";

            if (IDNameHash != NULL) {
                dbg.nospace() << IDNameHash->value(itemset[i]).toStdString().c_str()
                        << "("
                        << itemset[i]
                        << ")";
            }
            else
                dbg.nospace() << itemset[i];
        }

        return dbg.nospace();
    }

#endif

}
