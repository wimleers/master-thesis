#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QStringList>
#include <QtNetwork>
#include <QDateTime>
#include "typedefs.h"

namespace EpisodesParser {

    class Parser : public QObject {
        Q_OBJECT

    public:
        Parser();
        bool parse(const QString & fileName);

    signals:
    //    void parsedEpisode(Episode episode);

    public slots:

    protected:
        QHostAddress ipConvertor;
        QDateTime timeConvertor;
        EpisodeNameIDHash nameIDHash;
#ifdef DEBUG
        EpisodeIDNameHash IDNameHash;
#endif
        EpisodeID nextID;

        EpisodeID mapEpisodeNameToID(EpisodeName name);
    };

}
#endif // PARSER_H
