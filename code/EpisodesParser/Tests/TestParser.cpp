#include "TestParser.h"

void TestParser::init() {
    QFile logFile("episodes.log");
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        QFAIL("Could not create sample Episodes log file.");

    QTextStream out(&logFile);
    out << "218.56.155.59 [Sunday, 14-Nov-2010 06:27:03 +0100] \"?ets=css:203,headerjs:94,footerjs:500,domready:843,tabs:110,ToThePointShowHideChangelog:15,DrupalBehaviors:141,frontend:1547\" 200 \"http://driverpacks.net/driverpacks/windows/xp/x86/chipset/10.09\" \"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)\" \"driverpacks.net\"" << "\n"
        << "190.166.203.6 [Sunday, 14-Nov-2010 06:27:06 +0100] \"?ets=css:0,headerjs:588,footerjs:61,domready:680,tabs:1,ToThePointShowHideChangelog:0,DrupalBehaviors:1,frontend:998\" 200 \"http://driverpacks.net/\" \"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/534.7 (KHTML, like Gecko) Chrome/7.0.517.44 Safari/534.7\" \"driverpacks.net\"" << "\n"
        << "76.170.154.29 [Sunday, 14-Nov-2010 06:27:08 +0100] \"?ets=css:0,headerjs:41,footerjs:0,domready:822,tabs:1,ToThePointShowHideChangelog:0,DrupalBehaviors:1,frontend:990\" 200 \"http://driverpacks.net/driverpacks/windows/7/x86/graphics-b/10.07\" \"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.12) Gecko/20101026 Firefox/3.6.12\" \"driverpacks.net\"" << "\n"
        << "76.170.154.29 [Sunday, 14-Nov-2010 06:27:11 +0100] \"?ets=backend:439,css:61,headerjs:162,footerjs:0,domready:318,tabs:1,tableHeader:16,ToThePointShowHideChangelog:0,DrupalBehaviors:17,pageready:775,frontend:336,totaltime:775\" 200 \"http://driverpacks.net/driverpacks/windows/7/x86/graphics-b/10.07/drivers\" \"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.12) Gecko/20101026 Firefox/3.6.12\" \"driverpacks.net\"" << "\n"
        << "76.170.154.29 [Sunday, 14-Nov-2010 06:27:12 +0100] \"?ets=css:40,headerjs:90,footerjs:0,domready:223,tabs:1,ToThePointShowHideChangelog:0,DrupalBehaviors:1,frontend:382\" 200 \"http://driverpacks.net/driverpacks/windows/7/x86/graphics-b/10.07\" \"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.12) Gecko/20101026 Firefox/3.6.12\" \"driverpacks.net\"" << "\n";

    logFile.close();
}

void TestParser::cleanup() {
    QFile logFile("episodes.log");
    if (!logFile.remove())
        QFAIL("Could not delete sample Episodes log file.");
}

void TestParser::parse() {
    Parser parser;

    QVERIFY(parser.parse("episodes.log") == 5);
}

void TestParser::mapLineToEpisodesLogLine_data() {
    QTest::addColumn<QString>("line");
    QTest::addColumn<IPAddress>("ip");
    QTest::addColumn<Time>("time");
    QTest::addColumn<EpisodeList>("episodes");
    QTest::addColumn<HTTPStatus>("status");
    QTest::addColumn<DomainID>("domainID");

    QTest::newRow("sample line 1")
      << "218.56.155.59 [Sunday, 14-Nov-2010 06:27:03 +0100] \"?ets=css:203,headerjs:94,footerjs:500,domready:843,tabs:110,ToThePointShowHideChangelog:15,DrupalBehaviors:141,frontend:1547\" 200 \"http://driverpacks.net/driverpacks/windows/xp/x86/chipset/10.09\" \"Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1)\" \"driverpacks.net\""
      << (IPAddress) 3661142843
      << (Time) 1289712423
      << (
            EpisodeList()
            << Episode(0, 203)
            << Episode(1, 94)
            << Episode(2, 244)
            << Episode(3, 75)
            << Episode(4, 110)
            << Episode(5, 15)
            << Episode(6, 141)
            << Episode(7, 11)
      )
      << (HTTPStatus) 200
      << (DomainID) 0;

    QTest::newRow("sample line 2")
      << "190.166.203.6 [Sunday, 14-Nov-2010 06:27:06 +0100] \"?ets=css:0,headerjs:588,footerjs:61,domready:680,tabs:1,ToThePointShowHideChangelog:0,DrupalBehaviors:1,frontend:998\" 200 \"http://driverpacks.net/\" \"Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US) AppleWebKit/534.7 (KHTML, like Gecko) Chrome/7.0.517.44 Safari/534.7\" \"driverpacks.net\""
      << (IPAddress) 3198601990
      << (Time) 1289712426
      << (
            EpisodeList()
            << Episode(0, 0)
            << Episode(1, 588)
            << Episode(2, 61)
            << Episode(3, 680)
            << Episode(4, 1)
            << Episode(5, 0)
            << Episode(6, 1)
            << Episode(7, 998)
      )
      << (HTTPStatus) 200
      << (DomainID) 0;

    QTest::newRow("sample line 3")
      << "76.170.154.29 [Sunday, 14-Nov-2010 06:27:08 +0100] \"?ets=css:0,headerjs:41,footerjs:0,domready:822,tabs:1,ToThePointShowHideChangelog:0,DrupalBehaviors:1,frontend:990\" 200 \"http://driverpacks.net/driverpacks/windows/7/x86/graphics-b/10.07\" \"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.12) Gecko/20101026 Firefox/3.6.12\" \"driverpacks.net\""
      << (IPAddress) 1286248989
      << (Time) 1289712428
      << (
            EpisodeList()
            << Episode(0, 0)
            << Episode(1, 41)
            << Episode(2, 0)
            << Episode(3, 822)
            << Episode(4, 1)
            << Episode(5, 0)
            << Episode(6, 1)
            << Episode(7, 990)
      )
      << (HTTPStatus) 200
      << (DomainID) 0;

    QTest::newRow("sample line 4")
      << "76.170.154.29 [Sunday, 14-Nov-2010 06:27:11 +0100] \"?ets=backend:439,css:61,headerjs:162,footerjs:0,domready:318,tabs:1,tableHeader:16,ToThePointShowHideChangelog:0,DrupalBehaviors:17,pageready:775,frontend:336,totaltime:775\" 200 \"http://driverpacks.net/driverpacks/windows/7/x86/graphics-b/10.07/drivers\" \"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.12) Gecko/20101026 Firefox/3.6.12\" \"driverpacks.net\""
      << (IPAddress) 1286248989
      << (Time) 1289712431
      << (
            EpisodeList()
            << Episode(8, 439) // New episode name: "backend".
            << Episode(0, 61)
            << Episode(1, 162)
            << Episode(2, 0)
            << Episode(3, 318)
            << Episode(4, 1)
            << Episode(9, 16) // New episode name: "tableHeader".
            << Episode(5, 0)
            << Episode(6, 17)
            << Episode(10, 775) // New episode name: "pageReady".
            << Episode(7, 336)
            << Episode(11, 775) // New episode name: "totalTime".
      )
      << (HTTPStatus) 200
      << (DomainID) 0;

    QTest::newRow("sample line 5")
      << "76.170.154.29 [Sunday, 14-Nov-2010 06:27:12 +0100] \"?ets=css:40,headerjs:90,footerjs:0,domready:223,tabs:1,ToThePointShowHideChangelog:0,DrupalBehaviors:1,frontend:382\" 200 \"http://driverpacks.net/driverpacks/windows/7/x86/graphics-b/10.07\" \"Mozilla/5.0 (Windows; U; Windows NT 6.1; en-US; rv:1.9.2.12) Gecko/20101026 Firefox/3.6.12\" \"driverpacks.net\""
      << (IPAddress) 1286248989
      << (Time) 1289712432
      << (
            EpisodeList()
            << Episode(0, 40)
            << Episode(1, 90)
            << Episode(2, 0)
            << Episode(3, 223)
            << Episode(4, 1)
            << Episode(5, 0)
            << Episode(6, 1)
            << Episode(7, 382)
      )
      << (HTTPStatus) 200
      << (DomainID) 0;
}

void TestParser::mapLineToEpisodesLogLine() {
    static Parser p;
    static EpisodesLogLine e;
    QFETCH(QString, line);
    QFETCH(IPAddress, ip);
    QFETCH(Time, time);
    QFETCH(EpisodeList, episodes);
    QFETCH(HTTPStatus, status);
    QFETCH(DomainID, domainID);

    e = p.mapLineToEpisodesLogLine(line);

    QCOMPARE(e.ip, ip);
    QCOMPARE(e.time, time);
    QCOMPARE(e.episodes, episodes);
    QCOMPARE(e.status, status);
    QCOMPARE(e.domain.id, domainID);
}
