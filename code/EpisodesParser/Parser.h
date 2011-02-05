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
#include "QGeoIP.h"
#include "EpisodeDurationDiscretizer.h"
#include "typedefs.h"


namespace EpisodesParser {

    #define CHUNK_SIZE 4000

    class Parser : public QObject {
        Q_OBJECT

    public:
        Parser();
        static void clearCaches();
        int parse(const QString & fileName);

        // Processing logic.
        static EpisodesLogLine mapLineToEpisodesLogLine(const QString & line);
        static ExpandedEpisodesLogLine expandEpisodesLogLine(const EpisodesLogLine & line);
        static ExpandedEpisodesLogLine mapAndExpandToEpisodesLogLine(const QString & line);
        static QList<QStringList> mapExpandedEpisodesLogLineToTransactions(const ExpandedEpisodesLogLine & line);

    signals:
        void parsedChunk(QStringList chunk);
        void processedChunk(QList<QStringList> transactions);

    public slots:

    protected slots:
        void processParsedChunk(const QStringList & chunk);

    protected:
        // QHashes that are used to minimize memory usage.
        static EpisodeNameIDHash episodeNameIDHash;
        static DomainNameIDHash domainNameIDHash;
        static UAHierarchyDetailsIDHash uaHierarchyDetailsIDHash;
        static UAHierarchyIDDetailsHash uaHierarchyIDDetailsHash;
        static TYPE_hash_location_toID hash_location_toID;
        static TYPE_hash_location_fromID hash_location_fromID;
#ifdef DEBUG
        static EpisodeIDNameHash episodeIDNameHash;
        static DomainIDNameHash domainIDNameHash;
#endif

        static bool staticsInitialized;
        static QBrowsCap browsCap;
        static QGeoIP geoIP;
        static EpisodeDurationDiscretizer episodeDiscretizer;

        // Mutexes used to ensure thread-safety.
        static QMutex staticsInitializationMutex;
        static QMutex episodeHashMutex;
        static QMutex domainHashMutex;
        static QMutex uaHierarchyHashMutex;
        static QMutex mutex_hashAccess_location;
        static QMutex regExpMutex;
        static QMutex dateTimeMutex;

        // Methods to actually use the above QHashes.
        static EpisodeID mapEpisodeNameToID(EpisodeName name);
        static DomainID mapDomainNameToID(DomainName name);
        static UAHierarchyID mapUAHierarchyToID(UAHierarchyDetails ua);
        static LocationID hash_location_mapToID(const Location & location);
    };

}
#endif // PARSER_H
