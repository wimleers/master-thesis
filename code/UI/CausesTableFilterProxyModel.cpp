#include "CausesTableFilterProxyModel.h"


//------------------------------------------------------------------------------
// Public methods.

CausesTableFilterProxyModel::CausesTableFilterProxyModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

void CausesTableFilterProxyModel::setEpisodesColumn(int col) {
    this->episodesColumn = col;
    this->invalidateFilter();
}

void CausesTableFilterProxyModel::setCircumstancesColumn(int col) {
    this->circumstancesColumn = col;
    this->invalidateFilter();
}

void CausesTableFilterProxyModel::setEpisodeFilter(const QString & filter) {
    this->episodesFilter = QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString);
    this->invalidateFilter();
}

void CausesTableFilterProxyModel::setCircumstancesFilter(const QStringList & filter) {
    this->circumstancesFilter.clear();
    foreach (const QString & f, filter)
        this->circumstancesFilter.append(QRegExp(f, Qt::CaseInsensitive, QRegExp::Wildcard));
    this->invalidateFilter();
}


//------------------------------------------------------------------------------
// Protected methods.

bool CausesTableFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const {
    bool episodesColumnMatches = false;
    bool circumstancesColumnMatches = false;

    QModelIndex e = this->sourceModel()->index(sourceRow, this->episodesColumn, sourceParent);
    episodesColumnMatches = this->sourceModel()->data(e).toString().contains(this->episodesFilter);

    QModelIndex c = this->sourceModel()->index(sourceRow, this->circumstancesColumn, sourceParent);
    if (!this->circumstancesFilter.isEmpty()) {
        foreach (const QRegExp & regexp, this->circumstancesFilter) {
            circumstancesColumnMatches = this->sourceModel()->data(c).toString().contains(regexp);
            if (!circumstancesColumnMatches)
                break;
        }
    }
    else {
        // No circumstances filter, hence this column *always* matches.
        circumstancesColumnMatches = true;
    }

    return episodesColumnMatches && circumstancesColumnMatches;
}
