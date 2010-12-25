#ifndef PARSER_H
#define PARSER_H

#include <QObject>
#include <QFile>
#include <QRegExp>
#include <QTextStream>
#include <QStringList>
#include <QDateTime>
#include <QThread>
#include <QtConcurrentMap>
#include <QMutex>
#include "QBrowsCap.h"
#include "typedefs.h"

namespace EpisodesParser {

    #define CHUNK_SIZE 4000

    class Parser : public QObject {
        Q_OBJECT

    public:
        Parser();
        int parse(const QString & fileName);

        // Processing logic.
        static EpisodesLogLine mapLineToEpisodesLogLine(const QString & line);
        static ExpandedEpisodesLogLine expandEpisodesLogLine(const EpisodesLogLine & line);
        static ExpandedEpisodesLogLine mapAndExpandToEpisodesLogLine(const QString & line);

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

        static bool staticsInitialized;
        static QBrowsCap browsCap;

        // Mutexes used to ensure thread-safety.
        static QMutex staticsInitializationMutex;
        static QMutex episodeHashMutex;
        static QMutex domainHashMutex;
        static QMutex regExpMutex;
        static QMutex dateTimeMutex;

        // Methods to actually use the above QHashes.
        static EpisodeID mapEpisodeNameToID(EpisodeName name);
        static DomainID mapDomainNameToID(DomainName name);
    };

}
#endif // PARSER_H
