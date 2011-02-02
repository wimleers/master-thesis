#include "typedefs.h"

#ifdef DEBUG
QDebug operator<<(QDebug dbg, const Item & i) {
    dbg.nospace() << i.IDNameHash->value(i.id).toStdString().c_str()
                  << "("
                  << i.id
                  << ")="
                  << i.supportCount;

    return dbg.nospace();
}

QDebug operator<<(QDebug dbg, const Transaction &transaction) {
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

#endif
