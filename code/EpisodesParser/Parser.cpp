#include "Parser.h"

namespace EpisodesParser {
    Parser::Parser() {
        this->nextEpisodeID = this->nextDomainID = 0;
    }


    bool Parser::parse(const QString & fileName) {
        QFile file;
        EpisodesLogLine * parsedLine;
        QStringList episodesRaw, episodeParts;
        Episode episode;

        file.setFileName(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;

        QTextStream in(&file);
        QString line;

        int numLines = 0;

        while (!in.atEnd()) {
            line = in.readLine();
            QRegExp rx("((?:\\d{1,3}\\.){3}\\d{1,3}) \\[\\w+, ([^\\]]+)\\] \\\"\\?ets=([^\\\"]+)\\\" (\\d{3}) \\\"([^\\\"]+)\\\" \\\"([^\\\"]+)\\\" \\\"([^\\\"]+)\\\"");
            rx.indexIn(line);
            QStringList list = rx.capturedTexts();
            parsedLine = new EpisodesLogLine();

            // IP address.
            this->ipConvertor.setAddress(list[1]);
            parsedLine->ip = this->ipConvertor.toIPv4Address();
            this->ipConvertor.setAddress(parsedLine->ip);

            // Time.
            QString dateTime = list[2].left(20);
            int timezoneOffset = list[2].right(5).left(3).toInt();
            this->timeConvertor = QDateTime::fromString(dateTime, "dd-MMM-yyyy HH:mm:ss");
            // Mark this QDateTime as one with a certain offset from UTC, and
            // set that offset.
            this->timeConvertor.setTimeSpec(Qt::OffsetFromUTC);
            this->timeConvertor.setUtcOffset(timezoneOffset * 3600);
            // Convert this QDateTime to UTC.
            this->timeConvertor = this->timeConvertor.toUTC();
            // Store the UTC timestamp.
            parsedLine->time = this->timeConvertor.toTime_t();

            // Episode names and durations.
            episodesRaw = list[3].split(',');
            foreach (QString episodeRaw, episodesRaw) {
                episodeParts = episodeRaw.split(':');
                episode.id       = mapEpisodeNameToID(episodeParts[0]);
                episode.duration = episodeParts[1].toInt();
#ifdef DEBUG
                episode.IDNameHash = &this->episodeIDNameHash;
#endif
                parsedLine->episodes.append(episode);
            }

            // HTTP status code.
            parsedLine->status = list[4].toShort();

            // URL.
            parsedLine->url = list[5];

            // User-agent.
            parsedLine->ua = list[6];

            // Domain name.
            parsedLine->domain.id = this->mapDomainNameToID(list[7]);
#ifdef DEBUG
            parsedLine->domain.IDNameHash = &this->domainIDNameHash;
#endif


#ifdef DEBUG
            // Debug.
            parsedLine->episodeIDNameHash = &this->episodeIDNameHash;
            parsedLine->domainIDNameHash = &this->domainIDNameHash;
            qDebug() << *parsedLine;

            if (numLines++ == 2) {
                return true;
            }
#endif
        }

        return true;
    }


    //---------------------------------------------------------------------------
    // Protected methods.

    EpisodeID Parser::mapEpisodeNameToID(EpisodeName name) {
        if (!this->episodeNameIDHash.contains(name)) {
            EpisodeID id = this->nextEpisodeID++;
            this->episodeNameIDHash.insert(name, id);
#ifdef DEBUG
            this->episodeIDNameHash.insert(id, name);
#endif
        }

        return this->episodeNameIDHash[name];
    }

    DomainID Parser::mapDomainNameToID(DomainName name) {
        if (!this->domainNameIDHash.contains(name)) {
            DomainID id = this->nextDomainID++;
            this->domainNameIDHash.insert(name, id);
#ifdef DEBUG
            this->domainIDNameHash.insert(id, name);
#endif
        }

        return this->domainNameIDHash[name];
    }

}
