#include "Parser.h"

namespace EpisodesParser {
    EpisodeNameIDHash Parser::episodeNameIDHash;
    DomainNameIDHash Parser::domainNameIDHash;
    UAHierarchyDetailsIDHash Parser::uaHierarchyDetailsIDHash;
    UAHierarchyIDDetailsHash Parser::uaHierarchyIDDetailsHash;
#ifdef DEBUG
    EpisodeIDNameHash Parser::episodeIDNameHash;
    DomainIDNameHash Parser::domainIDNameHash;
#endif
    bool Parser::staticsInitialized = false;
    QBrowsCap Parser::browsCap;
    QGeoIP Parser::geoIP;
    QMutex Parser::staticsInitializationMutex;
    QMutex Parser::episodeHashMutex;
    QMutex Parser::domainHashMutex;
    QMutex Parser::uaHierarchyHashMutex;
    QMutex Parser::regExpMutex;
    QMutex Parser::dateTimeMutex;

    Parser::Parser() {
        // Only initialize some static members the first time an instance of
        // this class is created.
        this->staticsInitializationMutex.lock();
        if (!this->staticsInitialized) {
            this->browsCap.setCsvFile("/Users/wimleers/Desktop/browscap.csv");
            this->browsCap.setIndexFile("/Users/wimleers/Desktop/browscap-index.db");
            this->browsCap.buildIndex();

            this->geoIP.openDatabases("./data/GeoIPCity.dat", "./data/GeoIPASNum.dat");
        }
        this->staticsInitializationMutex.unlock();

        connect(this, SIGNAL(parsedChunk(QStringList)), SLOT(processParsedChunk(QStringList)));
    }

    /**
     * Parse the given Episodes log file.
     *
     * Emits a signal for every chunk (with CHUNK_SIZE lines). This signal can
     * then be used to process that chunk. This allows multiple chunks to be
     * processed in parallel (by using QtConcurrent), if that is desired.
     *
     * @param fileName
     *   The full path to an Episodes log file.
     * @return
     *   -1 if the log file could not be read. Otherwise: the number of lines
     *   parsed.
     */
    int Parser::parse(const QString & fileName) {
        QFile file;
        QStringList chunk;
        int numLines = 0;

        file.setFileName(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return -1;
        else {
            QTextStream in(&file);
            while (!in.atEnd()) {
                chunk.append(in.readLine());
                numLines++;
                if (chunk.size() == CHUNK_SIZE) {
                    emit parsedChunk(chunk);
                    chunk.clear();
                }
            }

            // Check if we have another chunk (with size < CHUNK_SIZE).
            if (chunk.size() > 0) {
                emit parsedChunk(chunk);
            }

            return numLines;
        }
    }


    //---------------------------------------------------------------------------
    // Protected methods.

    /**
     * Map an episode name to an episode ID. Generate a new ID when necessary.
     *
     * @param name
     *   Episode name.
     * @return
     *   The corresponding episode ID.
     *
     * Modifies a class variable upon some calls (i.e. when a new key must be
     * inserted in the QHash), hence we need to use a mutex to ensure thread
     * safety.
     */
    EpisodeID Parser::mapEpisodeNameToID(EpisodeName name) {
        if (!Parser::episodeNameIDHash.contains(name)) {
            Parser::episodeHashMutex.lock();

            EpisodeID id = Parser::episodeNameIDHash.size();
            Parser::episodeNameIDHash.insert(name, id);
#ifdef DEBUG
            Parser::episodeIDNameHash.insert(id, name);
#endif

            Parser::episodeHashMutex.unlock();
        }

        return Parser::episodeNameIDHash[name];
    }

    /**
     * Map a domain name to a domain ID. Generate a new ID when necessary.
     *
     * @param name
     *   Domain name.
     * @return
     *   The corresponding domain ID.
     *
     * Modifies a class variable upon some calls (i.e. when a new key must be
     * inserted in the QHash), hence we need to use a mutex to ensure thread
     * safety.
     */
    DomainID Parser::mapDomainNameToID(DomainName name) {
        if (!Parser::domainNameIDHash.contains(name)) {
            Parser::domainHashMutex.lock();

            DomainID id = Parser::domainNameIDHash.size();
            Parser::domainNameIDHash.insert(name, id);
#ifdef DEBUG
            Parser::domainIDNameHash.insert(id, name);
#endif

            Parser::domainHashMutex.unlock();
        }

        return Parser::domainNameIDHash[name];
    }

    /**
     * Map a UA hierarchy to a UA hierarchy ID. Generate a new ID when
     * necessary.
     *
     * @param ua
     *   UA hierarchy
     * @return
     *   The corresponding UA hierarchy ID.
     *
     * Modifies a class variable upon some calls (i.e. when a new key must be
     * inserted in the QHash), hence we need to use a mutex to ensure thread
     * safety.
     */
    UAHierarchyID Parser::mapUAHierarchyToID(UAHierarchyDetails ua) {
        if (!Parser::uaHierarchyDetailsIDHash.contains(ua)) {
            Parser::uaHierarchyHashMutex.lock();

            UAHierarchyID id = Parser::uaHierarchyDetailsIDHash.size();
            Parser::uaHierarchyDetailsIDHash.insert(ua, id);
            Parser::uaHierarchyIDDetailsHash.insert(id, ua);

            Parser::uaHierarchyHashMutex.unlock();
        }

        return Parser::uaHierarchyDetailsIDHash[ua];
    }

    /**
     * Map a line (raw string) to an EpisodesLogLine data structure.
     *
     * @param line
     *   Raw line, as read from the episodes log file.
     * @return
     *   Corresponding EpisodesLogLine data structure.
     */
    EpisodesLogLine Parser::mapLineToEpisodesLogLine(const QString & line) {
        static QDateTime timeConvertor;
        QStringList list, episodesRaw, episodeParts;
        QString dateTime;
        int timezoneOffset;
        Episode episode;
        EpisodesLogLine parsedLine;

        Parser::regExpMutex.lock();
        QRegExp rx("((?:\\d{1,3}\\.){3}\\d{1,3}) \\[\\w+, ([^\\]]+)\\] \\\"\\?ets=([^\\\"]+)\\\" (\\d{3}) \\\"([^\\\"]+)\\\" \\\"([^\\\"]+)\\\" \\\"([^\\\"]+)\\\"");
        rx.indexIn(line);
        list = rx.capturedTexts();
        Parser::regExpMutex.unlock();

        // IP address.
        parsedLine.ip.setAddress(list[1]);

        // Time.
        dateTime = list[2].left(20);
        timezoneOffset = list[2].right(5).left(3).toInt();
        Parser::dateTimeMutex.lock();
        timeConvertor = QDateTime::fromString(dateTime, "dd-MMM-yyyy HH:mm:ss");
        // Mark this QDateTime as one with a certain offset from UTC, and
        // set that offset.
        timeConvertor.setTimeSpec(Qt::OffsetFromUTC);
        timeConvertor.setUtcOffset(timezoneOffset * 3600);
        // Convert this QDateTime to UTC.
        timeConvertor = timeConvertor.toUTC();
        // Store the UTC timestamp.
        parsedLine.time = timeConvertor.toTime_t();
        Parser::dateTimeMutex.unlock();

        // Episode names and durations.
        episodesRaw = list[3].split(',');
        foreach (QString episodeRaw, episodesRaw) {
            episodeParts = episodeRaw.split(':');

            episode.id       = Parser::mapEpisodeNameToID(episodeParts[0]);
            episode.duration = episodeParts[1].toInt();
#ifdef DEBUG
            episode.IDNameHash = &Parser::episodeIDNameHash;
#endif
            parsedLine.episodes.append(episode);
        }

        // HTTP status code.
        parsedLine.status = list[4].toShort();

        // URL.
        parsedLine.url = list[5];

        // User-Agent.
        parsedLine.ua = list[6];

        // Domain name.
        parsedLine.domain.id = Parser::mapDomainNameToID(list[7]);
#ifdef DEBUG
        parsedLine.domain.IDNameHash = &domainIDNameHash;
#endif

#ifdef DEBUG
        /*
        parsedLine.episodeIDNameHash = &Parser::episodeIDNameHash;
        parsedLine.domainIDNameHash = &Parser::domainIDNameHash;
        qDebug() << parsedLine;
        */
#endif
        return parsedLine;
    }

    /**
     * Expand an EpisodesLogLine data structure to an ExpandedEpisodesLogLine
     * data structure, which contains the expanded (hierarchical) versions
     * of the ip and User Agent values. I.e. these expanded versions provide
     * a concept hierarchy.
     *
     * @param line
     *   EpisodesLogLine data structure.
     * return
     *   Corresponding ExpandedEpisodesLogLine data structure.
     */
    ExpandedEpisodesLogLine Parser::expandEpisodesLogLine(const EpisodesLogLine & line) {
        ExpandedEpisodesLogLine expandedLine;
        QGeoIPRecord geoIPRecord;
        QPair<bool, QBrowsCapRecord> browsCapResult;
        UAHierarchyDetails ua;

        // IP address hierarchy.
        geoIPRecord = Parser::geoIP.recordByAddr(line.ip);
        expandedLine.ip.ip        = line.ip;
        expandedLine.ip.continent = geoIPRecord.continentCode;
        expandedLine.ip.country   = geoIPRecord.country;
        expandedLine.ip.city      = geoIPRecord.city;
        expandedLine.ip.region    = geoIPRecord.region;
        expandedLine.ip.isp       = geoIPRecord.isp;

        // Time.
        expandedLine.time = line.time;

        // Episode name and durations.
        // @TODO: discretize to fast/acceptable/slow.
        expandedLine.episodes = line.episodes;

        // URL.
        expandedLine.url = line.url;

        // User-Agent hierarchy.
        browsCapResult = Parser::browsCap.matchUserAgent(line.ua);
        ua.platform              = browsCapResult.second.platform;
        ua.browser_name          = browsCapResult.second.browser_name;
        ua.browser_version       = browsCapResult.second.browser_version;
        ua.browser_version_major = browsCapResult.second.browser_version_major;
        ua.browser_version_minor = browsCapResult.second.browser_version_minor;
        ua.is_mobile             = browsCapResult.second.is_mobile;

        expandedLine.ua = Parser::mapUAHierarchyToID(ua);
        expandedLine.uaHierarchyIDDetailsHash = &Parser::uaHierarchyIDDetailsHash;

        return expandedLine;
    }


    ExpandedEpisodesLogLine Parser::mapAndExpandToEpisodesLogLine(const QString & line) {
        return Parser::expandEpisodesLogLine(Parser::mapLineToEpisodesLogLine(line));
    }

    //---------------------------------------------------------------------------
    // Protected slots.

    void Parser::processParsedChunk(const QStringList & chunk) {
        // This 100% concurrent approach fails, because QGeoIP still has
        // thread-safety issues. Hence, we only do the mapping from a QString
        // to an EpisodesLogLine concurrently for now.
        // QtConcurrent::blockingMapped(chunk, Parser::mapAndExpandToEpisodesLogLine);

        QList<EpisodesLogLine> mappedChunk = QtConcurrent::blockingMapped(chunk, Parser::mapLineToEpisodesLogLine);

        QList<ExpandedEpisodesLogLine> expandedChunk;
        EpisodesLogLine line;
        foreach (line, mappedChunk) {
            expandedChunk << Parser::expandEpisodesLogLine(line);
        }

        qDebug() << "Processed chunk! Size:" << expandedChunk.size();
    }
}
