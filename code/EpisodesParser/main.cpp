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
    parser->parse("/Users/wimleers/School/masterthesis/logs/driverpacks.net.episodes.log");
    cout << QString("Duration: %1 ms.").arg(timer.elapsed()) << endl;
}


