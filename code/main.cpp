#include "QCachingLocale.h"
#include "Parser.h"
#include "Analyst.h"
#include <QObject>
#include <QTime>

using namespace EpisodesParser;
using namespace Analytics;

int main(int argc, char *argv[]) {
    QCachingLocale cl;

    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    QTime timer;
    Parser * parser;
    Analyst * analyst;

    parser = new Parser();
    analyst = new Analyst(0.4, 0.8);
    QObject::connect(parser, SIGNAL(processedChunk(QList<QStringList>)), analyst, SLOT(analyzeTransactions(QList<QStringList>)));

    timer.start();
    int linesParsed = parser->parse("/Users/wimleers/School/masterthesis/logs/driverpacks.net.episodes.log");
    int timePassed = timer.elapsed();
    cout << QString("Duration: %1 ms. Parsed %2 lines. That's %3 lines/ms or %4 ms/line.")
            .arg(timePassed)
            .arg(linesParsed)
            .arg((float)linesParsed/timePassed)
            .arg((float)timePassed/linesParsed)
            << endl;

    delete parser;


    QTextStream in(stdin);
    forever {
        QString line = in.readLine();
        if (!line.isNull())
            break;
    }
}


