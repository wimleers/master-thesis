#ifndef EPISODEDURATIONDISCRETIZER_H
#define EPISODEDURATIONDISCRETIZER_H

#include <QString>
#include <QList>
#include <QMap>
#include <QFile>
#include <QTextStream>
#include "typedefs.h"

namespace EpisodesParser {
    class EpisodeDurationDiscretizer {
    public:
        EpisodeDurationDiscretizer();
        bool parseCsvFile(const QString & csvFile);
        EpisodeSpeed mapToSpeed(const EpisodeName & name, const EpisodeDuration & duration) const;

    private:
        QString csvFile;
        QMap<EpisodeName, QMap<EpisodeDuration, EpisodeSpeed> > thresholds;
    };
}

#endif // EPISODEDURATIONDISCRETIZER_H
