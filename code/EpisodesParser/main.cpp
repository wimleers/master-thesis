#include "Parser.h"
#include "QCachingLocale.h"

using namespace EpisodesParser;

int main(int argc, char *argv[]) {
    QCachingLocale cl;

    QTextStream cout(stdout);
    QTextStream cerr(stderr);
    Parser * parser;
    QTime timer;


    parser = new Parser();

    timer.start();
    int linesParsed = parser->parse("/Users/wimleers/School/masterthesis/logs/driverpacks.net.episodes.log");
    cout << QString("Duration: %1 ms. Parsed %2 lines.").arg(timer.elapsed()).arg(linesParsed) << endl;
}


