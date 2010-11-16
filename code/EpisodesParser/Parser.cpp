#include "Parser.h"

namespace EpisodesParser {
    Parser::Parser() {

    }


    bool Parser::parse(const QString & fileName) {
        QFile file;
        EpisodesLogLine * parsedLine;

        file.setFileName(fileName);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
            return false;

        QTextStream in(&file);
        QString line;

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

#ifdef DEBUG
            // Debug.
            qDebug() << line;
            int i = 0;
            foreach (QString part, list) {
                qDebug() << i++ << ")   " <<  part;
            }
            qDebug() << *parsedLine;

            return true;
#endif
        }

        return true;
    }

}
