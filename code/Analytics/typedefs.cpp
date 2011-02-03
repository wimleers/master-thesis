#include "typedefs.h"

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

    QDebug operator<<(QDebug dbg, const AssociationRule & associationRule) {
        dbg.nospace() << "{";
        for (int i = 0; i < associationRule.antecedent.size(); i++) {
            if (i > 0)
                dbg.nospace() << ", ";
            dbg.nospace() << associationRule.antecedent[i];
        }
        dbg.nospace() << "} => {";
        for (int i = 0; i < associationRule.consequent.size(); i++) {
            if (i > 0)
                dbg.nospace() << ", ";
            dbg.nospace() << associationRule.consequent[i];
        }
        dbg.nospace() << "}";

        dbg.nospace() << " (conf=" << associationRule.confidence << ")";

        return dbg.nospace();
    }
#endif

}
