#ifndef CONCEPTHIERARCHYCOMPLETER
#define CONCEPTHIERARCHYCOMPLETER

#include <QCompleter>
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QStringList>
#include <QWidget>
#include <QLineEdit>

class ConceptHierarchyCompleter : public QCompleter {

    Q_OBJECT

public:
    ConceptHierarchyCompleter(QObject * parent = NULL);
    ConceptHierarchyCompleter(QAbstractItemModel * model, QObject * parent = NULL);

protected:
    void init();

    QStringList splitPath(const QString & path) const;
    QString pathFromIndex(const QModelIndex &index) const;

    QString lineageSeparator;
    QString entrySeparator;
};

#endif // CONCEPTHIERARCHYCOMPLETER
