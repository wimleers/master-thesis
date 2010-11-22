#include "Parser.h"

namespace EpisodesParser {
    EpisodeNameIDHash Parser::episodeNameIDHash;
    DomainNameIDHash Parser::domainNameIDHash;
    EpisodeIDNameHash Parser::episodeIDNameHash;
    DomainIDNameHash Parser::domainIDNameHash;
    QMutex Parser::episodeHashMutex;
    QMutex Parser::domainHashMutex;

    Parser::Parser() {
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
     * Map a line (raw string) to an EpisodesLogLine data structure.
     *
     * @param line
     *   Raw line, as read from the episodes log file.
     * @return
     *   Corresponding EpisodesLogLine data structure.
     */
    EpisodesLogLine Parser::mapLineToEpisodesLogLine(const QString & line) {
        static QHostAddress ipConvertor;
        static QDateTime timeConvertor;
        QRegExp rx("((?:\\d{1,3}\\.){3}\\d{1,3}) \\[\\w+, ([^\\]]+)\\] \\\"\\?ets=([^\\\"]+)\\\" (\\d{3}) \\\"([^\\\"]+)\\\" \\\"([^\\\"]+)\\\" \\\"([^\\\"]+)\\\"");
        QStringList list, episodesRaw, episodeParts;
        QString dateTime;
        int timezoneOffset;
        Episode episode;
        EpisodesLogLine parsedLine;

        rx.indexIn(line);
        list = rx.capturedTexts();

        // IP address.
        ipConvertor.setAddress(list[1]);
        parsedLine.ip = ipConvertor.toIPv4Address();

        // Time.
        dateTime = list[2].left(20);
        timezoneOffset = list[2].right(5).left(3).toInt();
        timeConvertor = QDateTime::fromString(dateTime, "dd-MMM-yyyy HH:mm:ss");
        // Mark this QDateTime as one with a certain offset from UTC, and
        // set that offset.
        timeConvertor.setTimeSpec(Qt::OffsetFromUTC);
        timeConvertor.setUtcOffset(timezoneOffset * 3600);
        // Convert this QDateTime to UTC.
        timeConvertor = timeConvertor.toUTC();
        // Store the UTC timestamp.
        parsedLine.time = timeConvertor.toTime_t();

        // Episode names and durations.
        episodesRaw = list[3].split(',');
        foreach (QString episodeRaw, episodesRaw) {
            episodeParts = episodeRaw.split(':');

            episode.id       = mapEpisodeNameToID(episodeParts[0]);
            episode.duration = episodeParts[1].toInt();
#ifdef DEBUG
            episode.IDNameHash = &episodeIDNameHash;
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
        parsedLine.episodeIDNameHash = &this->episodeIDNameHash;
        parsedLine.domainIDNameHash = &this->domainIDNameHash;
        qDebug() << parsedLine;
        */
#endif

        return parsedLine;
    }


    //---------------------------------------------------------------------------
    // Protected slots.

    void Parser::processParsedChunk(const QStringList & chunk) {
        QList<EpisodesLogLine> processedChunk = QtConcurrent::blockingMapped(chunk, Parser::mapLineToEpisodesLogLine);
    }
}
