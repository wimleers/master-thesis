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
#include <QMutexLocker>

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
        static void initParserHelpers(const QString & browsCapCSV,
                                      const QString & browsCapIndex,
                                      const QString & geoIPCityDB,
                                      const QString & geoIPISPDB,
                                      const QString & episodeDiscretizerCSV);
        static void clearParserHelperCaches();
        int parse(const QString & fileName);

        // Processing logic.
        static EpisodesLogLine mapLineToEpisodesLogLine(const QString & line);
        static ExpandedEpisodesLogLine expandEpisodesLogLine(const EpisodesLogLine & line);
        static ExpandedEpisodesLogLine mapAndExpandToEpisodesLogLine(const QString & line);
        static QList<QStringList> mapExpandedEpisodesLogLineToTransactions(const ExpandedEpisodesLogLine & line);

    signals:
        void processedChunk(QList<QStringList> transactions, double transactionsPerEvent);

    protected slots:
        void processBatch(const QList<EpisodesLogLine> batch);

    protected:
        void processParsedChunk(const QStringList & chunk);
        // QHashes that are used to minimize memory usage.
        static EpisodeNameIDHash episodeNameIDHash;
        static EpisodeIDNameHash episodeIDNameHash;
        static DomainNameIDHash domainNameIDHash;
        static DomainIDNameHash domainIDNameHash;
        static UAHierarchyDetailsIDHash uaHierarchyDetailsIDHash;
        static UAHierarchyIDDetailsHash uaHierarchyIDDetailsHash;
        static LocationToIDHash locationToIDHash;
        static LocationFromIDHash locationFromIDHash;

        static bool parserHelpersInitialized;
        static QBrowsCap browsCap;
        static QGeoIP geoIP;
        static EpisodeDurationDiscretizer episodeDiscretizer;

        // Mutexes used to ensure thread-safety.
        static QMutex parserHelpersInitMutex;
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
        static LocationID mapLocationToID(const Location & location);
    };

}
#endif // PARSER_H
