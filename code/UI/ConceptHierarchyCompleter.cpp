#include "ConceptHierarchyCompleter.h"

ConceptHierarchyCompleter::ConceptHierarchyCompleter(QObject * parent) :
    QCompleter(parent)
{
    this->init();
}

ConceptHierarchyCompleter::ConceptHierarchyCompleter(QAbstractItemModel * model, QObject * parent)
    :QCompleter(model, parent)
{
    this->init();
}


//------------------------------------------------------------------------------
// Protected methods.

void ConceptHierarchyCompleter::init() {
    this->lineageSeparator = ":";
    this->entrySeparator = ", ";
}

/**
 * Override of splitPath().
 */
QStringList ConceptHierarchyCompleter::splitPath(const QString & path) const {
    return path.split(this->entrySeparator).last() // Get the last path.
               .split(this->lineageSeparator);     // And split it.
}

/**
 * Override of pathFromIndex().
 */
QString ConceptHierarchyCompleter::pathFromIndex(const QModelIndex &index) const {
    // Calculate the last path.
    QStringList lineage;
    for (QModelIndex i = index; i.isValid(); i = i.parent())
        lineage.prepend(model()->data(i, completionRole()).toString());
    QString path = lineage.join(this->lineageSeparator);

    // Make sure that previously entered paths are also kept.
    QLineEdit * widget = (QLineEdit *) this->widget();
    QStringList list = widget->text().split(this->entrySeparator);
    if (list.size() > 1) {
        // Delete the last path in the list, which was uncomplete.
        list.removeLast();
        // Add the *completed* last path again at the end of the list.
        list.append(path);
        // Merge all paths together again.
        path = list.join(this->entrySeparator);
    }

    return path;
}
