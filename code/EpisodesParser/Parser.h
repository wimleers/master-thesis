#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QStringList>
#include <QtNetwork>
#include <QDateTime>
#include <QThread>
#include <QtConcurrentMap>
#include <QMutex>
#include "typedefs.h"

namespace EpisodesParser {

    #define CHUNK_SIZE 1000

    class Parser : public QObject {
        Q_OBJECT

    public:
        Parser();
        int parse(const QString & fileName);

        // Processing logic.
        static EpisodesLogLine mapLineToEpisodesLogLine(const QString & line);

    signals:
        void parsedChunk(QStringList chunk);
    //    void parsedEpisode(Episode episode);

    public slots:

    protected slots:
        void processParsedChunk(const QStringList & chunk);

    protected:
        // QHashes that are used to minimize memory usage.
        static EpisodeNameIDHash episodeNameIDHash;
        static DomainNameIDHash domainNameIDHash;
#ifdef DEBUG
        static EpisodeIDNameHash episodeIDNameHash;
        static DomainIDNameHash domainIDNameHash;
#endif

        // Mutexes used to ensure thread-safety.
        static QMutex episodeHashMutex;
        static QMutex domainHashMutex;

        // Methods to actually use the above QHashes.
        static EpisodeID mapEpisodeNameToID(EpisodeName name);
        static DomainID mapDomainNameToID(DomainName name);
    };

}
#endif // PARSER_H
