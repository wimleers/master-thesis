#include "Parser.h"

namespace EpisodesParser {
    EpisodeNameIDHash Parser::episodeNameIDHash;
    EpisodeIDNameHash Parser::episodeIDNameHash;
    DomainNameIDHash Parser::domainNameIDHash;
    DomainIDNameHash Parser::domainIDNameHash;
    UAHierarchyDetailsIDHash Parser::uaHierarchyDetailsIDHash;
    UAHierarchyIDDetailsHash Parser::uaHierarchyIDDetailsHash;
    LocationToIDHash   Parser::locationToIDHash;
    LocationFromIDHash Parser::locationFromIDHash;
    bool Parser::parserHelpersInitialized = false;

    QBrowsCap Parser::browsCap;
    QGeoIP Parser::geoIP;
    EpisodeDurationDiscretizer Parser::episodeDiscretizer;

    QMutex Parser::parserHelpersInitMutex;
    QMutex Parser::episodeHashMutex;
    QMutex Parser::domainHashMutex;
    QMutex Parser::uaHierarchyHashMutex;
    QMutex Parser::mutex_hashAccess_location;
    QMutex Parser::regExpMutex;
    QMutex Parser::dateTimeMutex;

    Parser::Parser() {
        Parser::parserHelpersInitMutex.lock();
        if (!Parser::parserHelpersInitialized)
            qFatal("Call Parser::initParserHelper()  before creating Parser instances.");
        Parser::parserHelpersInitMutex.unlock();
    }

    void Parser::initParserHelpers(const QString & browsCapCSV,
                                   const QString & browsCapIndex,
                                   const QString & geoIPCityDB,
                                   const QString & geoIPISPDB,
                                   const QString & episodeDiscretizerCSV)
    {
        Parser::parserHelpersInitMutex.lock();
        if (!Parser::parserHelpersInitialized) {
            // About 1.5 MB of permanent memory consumption.
            Parser::browsCap.setCsvFile(browsCapCSV);
            Parser::browsCap.setIndexFile(browsCapIndex);
            Parser::browsCap.buildIndex();

            // About 25 MB of permanent memory consumption.
            Parser::geoIP.openDatabases(geoIPCityDB, geoIPISPDB);

            // No significant permanent memory consumption.
            Parser::episodeDiscretizer.parseCsvFile(episodeDiscretizerCSV);

            Parser::parserHelpersInitialized = true;
        }
        Parser::parserHelpersInitMutex.unlock();
    }

    /**
     * Clear parser helpers' caches.
     *
     * This clears as many parser helpers' caches as possible:
     * - QGeoIP's databases are closed (but this unfortunately doesn't affect
     *   memory usage significantly, due to problems with the underlying
     *   libGeoIP library of MaxMind)
     * - QBrowsCap's in-memory cache is cleared
     *
     * Call this function whenever the Parser will not be used for long
     * periods of time.
     */
    void Parser::clearParserHelperCaches() {
        Parser::parserHelpersInitMutex.lock();
        if (Parser::parserHelpersInitialized) {
            Parser::geoIP.closeDatabases();
            Parser::browsCap.resetCache();

            Parser::parserHelpersInitialized = false;
        }
        Parser::parserHelpersInitMutex.unlock();
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
                    this->processParsedChunk(chunk);
                    chunk.clear();
                }
            }

            // Check if we have another chunk (with size < CHUNK_SIZE).
            if (chunk.size() > 0) {
                this->processParsedChunk(chunk);
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
            Parser::episodeIDNameHash.insert(id, name);

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
     * Map a Location to a Location ID. Generate a new ID when necessary.
     *
     * @param location
     *   Location.
     * @return
     *   The corresponding location ID.
     *
     * Modifies a class variable upon some calls (i.e. when a new key must be
     * inserted in the QHash), hence we need to use a mutex to ensure thread
     * safety.
     */
    LocationID Parser::mapLocationToID(const Location & location) {
        if (!Parser::locationToIDHash.contains(location)) {
            Parser::mutex_hashAccess_location.lock();

            LocationID id = Parser::locationToIDHash.size();
            Parser::locationToIDHash.insert(location, id);
            Parser::locationFromIDHash.insert(id, location);

            Parser::mutex_hashAccess_location.unlock();
        }

        return Parser::locationToIDHash[location];
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
        Location location;
        UAHierarchyDetails ua;

        // IP address hierarchy.
        geoIPRecord = Parser::geoIP.recordByAddr(line.ip);
        location.continent = geoIPRecord.continentCode;
        location.country   = geoIPRecord.country;
        location.city      = geoIPRecord.city;
        location.region    = geoIPRecord.region;
        location.isp       = geoIPRecord.isp;

        expandedLine.location = Parser::mapLocationToID(location);
        expandedLine.locationFromIDHash = &Parser::locationFromIDHash;

        // Time.
        expandedLine.time = line.time;

        // Episode name and durations.
        // @TODO: discretize to fast/acceptable/slow.
        expandedLine.episodes = line.episodes;

        // HTTP status code.
        expandedLine.status = line.status;

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

    QList<QStringList> Parser::mapExpandedEpisodesLogLineToTransactions(const ExpandedEpisodesLogLine & line) {
        QList<QStringList> transactions;
        QStringList itemList;
        itemList << QString("url:") + QString(line.url)
                 << Parser::locationFromIDHash.value(line.location).generateAssociationRuleItems()
                 << Parser::uaHierarchyIDDetailsHash.value(line.ua).generateAssociationRuleItems();

        // Only include the HTTP status code in the transaction if it's not a 200 status.
        // TODO: improve performance of this: by simply omitting this check, the entire process becomes 5% faster!
        if (line.status != 200)
            itemList << QString("status:") + QString::number(line.status);

        Episode episode;
        EpisodeName episodeName;
        QStringList transaction;
        foreach (episode, line.episodes) {
            episodeName = Parser::episodeIDNameHash[episode.id];
            transaction << QString("episode:") + episodeName
                        << QString("duration:") + Parser::episodeDiscretizer.mapToSpeed(episodeName, episode.duration)
                        // Append the shared items.
                        << itemList;
            transactions << transaction;
            transaction.clear();
        }

        return transactions;
    }


    //---------------------------------------------------------------------------
    // Protected slots.

    void Parser::processBatch(const QList<EpisodesLogLine> batch) {
        double transactionsPerEvent;
#ifdef DEBUG
        uint items = 0;
#endif

        // This 100% concurrent approach fails, because QGeoIP still has
        // thread-safety issues. Hence, we only do the mapping from a QString
        // to an EpisodesLogLine concurrently for now.
        // QtConcurrent::blockingMapped(chunk, Parser::mapAndExpandToEpisodesLogLine);

        // Perform the expanding of the EpisodesLogLines sequentially.
        // Reason: see above.
        QList<ExpandedEpisodesLogLine> expandedChunk;
        EpisodesLogLine line;
        foreach (line, batch) {
            expandedChunk << Parser::expandEpisodesLogLine(line);
        }

        // Perform the mapping from ExpandedEpisodesLogLines to groups of
        // transactions concurrently
//        QList< QList<QStringList> > groupedTransactions = QtConcurrent::blockingMapped(expandedChunk, Parser::mapExpandedEpisodesLogLineToTransactions);
        QList< QList<QStringList> > groupedTransactions;
        ExpandedEpisodesLogLine expLine;
        foreach (expLine, expandedChunk) {
            groupedTransactions << Parser::mapExpandedEpisodesLogLineToTransactions(expLine);
        }

        // Perform the merging of transaction groups into a single list of
        // transactions sequentially (impossible to do concurrently).
        QList<QStringList> transactions;
        QList<QStringList> transactionGroup;
        foreach (transactionGroup, groupedTransactions) {
            transactions.append(transactionGroup);
#ifdef DEBUG
            foreach (const QStringList & transaction, transactionGroup)
                items += transaction.size();
#endif
        }

        transactionsPerEvent = ((double) transactions.size()) / batch.size();

        qDebug() << "Processed batch of" << batch.size() << "lines!"
                 << "Transactions generated:" << transactions.size() << "."
                 << "(" << transactionsPerEvent << "transactions/event)"
#ifdef DEBUG
                 << "Avg. transaction length:" << 1.0 * items / transactions.size() << "."
                 << "(" << items << "items in total)"
#endif
                 << "Events occurred between"
                 << QDateTime::fromTime_t(batch.first().time).toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str()
                 << "and"
                 << QDateTime::fromTime_t(batch.last().time).toString("yyyy-MM-dd hh:mm:ss").toStdString().c_str();
    //---------------------------------------------------------------------------
    // Protected methods.

    void Parser::processParsedChunk(const QStringList & chunk) {
        static unsigned int quarterID = 0;
        static QList<EpisodesLogLine> batch;

        qDebug() << "STARTING CHUNK, remaining lines in previous batch" << batch.size();

        // Perform the mapping from strings to EpisodesLogLine concurrently.
//        QList<EpisodesLogLine> mappedChunk = QtConcurrent::blockingMapped(chunk, Parser::mapLineToEpisodesLogLine);
        QString rawLine;
        EpisodesLogLine line;
        foreach (rawLine, chunk) {
            line = Parser::mapLineToEpisodesLogLine(rawLine);

            // Create a batch for each quarter (900 seconds) and process it.
            if (line.time / 900 > quarterID) {
                quarterID = line.time / 900;
                if (!batch.isEmpty())
                    this->processBatch(batch);
                batch.clear();
            }

            batch.append(line);
        }

        qDebug() << "Processed chunk of" << CHUNK_SIZE << "lines!";
    }
}
