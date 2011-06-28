#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    this->parsing = false;
    this->totalPatternsExaminedWhileMining = 0;
    this->totalParsingDuration = 0;
    this->totalAnalyzingDuration = 0;
    this->totalMiningDuration = 0;

    // Logic + connections.
    this->initLogic();
    this->connectLogic();
    this->initUI();
    this->connectUI();
    this->assignLogicToThreads();
}

MainWindow::~MainWindow() {
    delete this->parser;
    delete this->analyst;
}

//------------------------------------------------------------------------------
// Public slots

void MainWindow::wakeParser() {
    this->parser->continueParsing();
}

void MainWindow::updateParsingStatus(bool parsing) {
    QMutexLocker(&this->statusMutex);
    this->parsing = parsing;
    this->updateStatus();
    this->menuFileImport->setEnabled(!parsing);
    if (!parsing)
        this->mineOrCompare();
}

void MainWindow::updateParsingDuration(int duration) {
    QMutexLocker(&this->statusMutex);
    this->totalParsingDuration += duration;
    this->status_performance_parsing->setText(
                QString("%1 s (%2 page views/s)")
                .arg(QString::number(this->totalParsingDuration / 1000.0, 'f', 2))
                .arg(QString::number(this->totalPageViews / (this->totalParsingDuration / 1000.0), 'f', 0))
    );
}

void MainWindow::updateAnalyzingDuration(int duration) {
    QMutexLocker(&this->statusMutex);
    this->totalAnalyzingDuration += duration;
    this->status_performance_analyzing->setText(
                QString("%1 s (%2 episodes/s)")
                .arg(QString::number(this->totalAnalyzingDuration / 1000.0, 'f', 2))
                .arg(QString::number(this->totalTransactions / (this->totalAnalyzingDuration / 1000.0), 'f', 0))
    );
}

void MainWindow::updateMiningDuration(int duration) {
    QMutexLocker(&this->statusMutex);
    this->totalMiningDuration += duration;
    this->status_performance_mining->setText(
                QString("%1 s (%2 patterns/s)")
                .arg(QString::number(this->totalMiningDuration / 1000.0, 'f', 2))
                .arg(QString::number(this->totalPatternsExaminedWhileMining / (this->totalMiningDuration / 1000.0), 'f', 0))
    );
}

void MainWindow::updateAnalyzingStatus(bool analyzing, Time start, Time end, int numPageViews, int numTransactions) {
    // Analysis started.
    if (analyzing) {
        this->updateStatus(
                    tr("Processing %1 page views (%2 transactions) between %3 and %4")
                    .arg(numPageViews)
                    .arg(numTransactions)
                    .arg(QDateTime::fromTime_t(start).toString("yyyy-MM-dd hh:mm:ss"))
                    .arg(QDateTime::fromTime_t(end).toString("yyyy-MM-dd hh:mm:ss"))
        );
    }
    // Analysis completed.
    else
        this->updateStatus();
}

void MainWindow::updateMiningStatus(bool mining) {
    if (mining)
        this->updateStatus("Mining association rules");
    else
        this->updateStatus();
}

void MainWindow::updateAnalyzingStats(Time start, Time end, int pageViews, int transactions, int uniqueItems, int frequentItems, int patternTreeSize) {
    this->statusMutex.lock();
    this->totalPageViews = pageViews;
    this->totalTransactions = transactions;
    this->patternTreeSize = patternTreeSize;
    this->startTime = start;
    this->endTime = end;
    this->statusMutex.unlock();

    this->status_measurements_startDate->setText(QDateTime::fromTime_t(start).toString("yyyy-MM-dd hh:mm:ss"));
    this->status_measurements_endDate->setText(QDateTime::fromTime_t(end).toString("yyyy-MM-dd hh:mm:ss"));

    this->status_measurements_pageViews->setText(QString::number(pageViews));
    this->status_measurements_episodes->setText(QString::number(transactions));

    this->status_mining_uniqueItems->setText(
                QString("%1 (%2 MB)")
                .arg(QString::number(uniqueItems))
                .arg(QString::number(uniqueItems * (4 + STATS_ITEM_ESTIMATED_AVG_BYTES) / 1000.0 / 1000.0, 'f', 2))
    );
    this->status_mining_frequentItems->setText(
                QString("%1 (%2 MB)")
                .arg(QString::number(frequentItems))
                .arg(QString::number(frequentItems * (4 + STATS_ITEM_ESTIMATED_AVG_BYTES) / 1000.0 / 1000.0, 'f', 2))
    );
    this->status_mining_patternTree->setText(
                QString("%1 (%2 MB)")
                .arg(QString::number(patternTreeSize))
                .arg(QString::number(((12 + (patternTreeSize * (STATS_TILTED_TIME_WINDOW_BYTES + STATS_FPNODE_ESTIMATED_CHILDREN_AVG_BYTES))) / 1000.0 / 1000.0), 'f', 2))
    );
}

void MainWindow::minedRules(uint from, uint to, QList<Analytics::AssociationRule> associationRules, Analytics::SupportCount eventsInTimeRange) {
    Time latestAnalyzedTime = this->endTime - (this->endTime % 900) + 900;
    Time endTime = latestAnalyzedTime - (Analytics::TiltedTimeWindow::quarterDistanceToBucket(from, false) * 900);
    Time startTime = latestAnalyzedTime - (Analytics::TiltedTimeWindow::quarterDistanceToBucket(to, true) * 900);
    if (startTime < this->startTime)
        startTime = this->startTime;

    this->statusMutex.lock();
    this->totalPatternsExaminedWhileMining += this->patternTreeSize;
    this->causesDescription->setText(
                QString(tr("%1 causes mined from %2 page views (from %3 until %4)"))
                .arg(associationRules.size())
                .arg(eventsInTimeRange)
                .arg(QDateTime::fromTime_t(startTime).toString("yyyy-MM-dd hh:mm:ss"))
                .arg(QDateTime::fromTime_t(endTime).toString("yyyy-MM-dd hh:mm:ss"))
    );
    this->statusMutex.unlock();

    QAbstractItemModel * oldModel = this->causesTableProxyModel->sourceModel();
    if (oldModel == (QObject *) NULL)
        delete oldModel;

    QStandardItemModel * model = new QStandardItemModel(associationRules.size(), 4, this);

    QStringList headerLabels;
    headerLabels << tr("Episode") << tr("Circumstances") << tr("% slow") << tr("# slow");
    model->setHorizontalHeaderLabels(headerLabels);

    int row = 0;
    QPair<Analytics::ItemName, Analytics::ItemNameList> antecedent;
    foreach (Analytics::AssociationRule rule, associationRules) {
        antecedent = this->analyst->extractEpisodeFromItemset(rule.antecedent);
        QString episode = antecedent.first.section(':', 1);
        QStandardItem * episodeItem = new QStandardItem(episode);
        episodeItem->setData(episode.toUpper(), Qt::UserRole);
        model->setItem(row, 0, episodeItem);

        QString circumstances = ((QStringList) antecedent.second).join(", ");
        QStandardItem * circumstancesItem = new QStandardItem(circumstances);
        circumstancesItem->setData(circumstances, Qt::UserRole);
        model->setItem(row, 1, circumstancesItem);

        QStandardItem * confidenceItem = new QStandardItem(QString("%1%").arg(QString::number(rule.confidence * 100, 'f', 2)));
        confidenceItem->setData(rule.confidence, Qt::UserRole);
        model->setItem(row, 2, confidenceItem);

        double relOccurrences = rule.support * 100.0 / eventsInTimeRange;
        QStandardItem * occurrencesItem = new QStandardItem(
                    QString("%1 (%2%)")
                    .arg(QString::number(rule.support))
                    .arg(QString::number(relOccurrences, 'f', 2))
        );
        occurrencesItem->setData(rule.support, Qt::UserRole);
        model->setItem(row, 3, occurrencesItem);

        row++;
    }
    model->setSortRole(Qt::UserRole);

    this->causesTableProxyModel->setSourceModel(model);
    this->causesTable->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
}

void MainWindow::comparedMinedRules(uint fromOlder, uint toOlder,
                        uint fromNewer, uint toNewer,
                        QList<Analytics::AssociationRule> intersectedRules,
                        QList<Analytics::AssociationRule> olderRules,
                        QList<Analytics::AssociationRule> newerRules,
                        QList<Analytics::AssociationRule> comparedRules,
                        QList<Analytics::Confidence> confidenceVariance,
                        QList<float> supportVariance,
                        Analytics::SupportCount eventsInIntersectedTimeRange,
                        Analytics::SupportCount eventsInOlderTimeRange,
                        Analytics::SupportCount eventsInNewerTimeRange)
{
    // Currently unused in the causes description.
    /*
    uint from = (fromOlder <= fromNewer) ? fromOlder : fromNewer;
    uint to = (toOlder >= toNewer) ? toOlder : toNewer;

    Time latestAnalyzedTime = this->endTime - (this->endTime % 900) + 900;
    Time endTime = latestAnalyzedTime - (Analytics::TiltedTimeWindow::quarterDistanceToBucket(from, false) * 900);
    Time startTime = latestAnalyzedTime - (Analytics::TiltedTimeWindow::quarterDistanceToBucket(to, true) * 900);
    if (startTime < this->startTime)
        startTime = this->startTime;
    */
    Q_UNUSED(fromOlder)
    Q_UNUSED(toOlder)
    Q_UNUSED(fromNewer)
    Q_UNUSED(toNewer)

    this->statusMutex.lock();
    this->totalPatternsExaminedWhileMining += this->patternTreeSize * 2;
    this->causesDescription->setText(
                QString(tr("Mined %1 and %2 causes, of which %3 occurred in both time granularities, thus totalling %4 unique causes"))
                .arg(olderRules.size())
                .arg(newerRules.size())
                .arg(intersectedRules.size())
                .arg(comparedRules.size())
    );
    this->statusMutex.unlock();

    QAbstractItemModel * oldModel = this->causesTableProxyModel->sourceModel();
    if (oldModel == (QObject *) NULL)
        delete oldModel;

    QStandardItemModel * model = new QStandardItemModel(comparedRules.size(), 6, this);

    QStringList headerLabels;
    headerLabels << tr("Episode") << tr("Circumstances") << tr("% slow") << tr("% slow change") << tr("# slow") << tr("# slow change");
    model->setHorizontalHeaderLabels(headerLabels);

    int row = 0;
    QPair<Analytics::ItemName, Analytics::ItemNameList> antecedent;
    for (int i = 0; i < comparedRules.size(); i++) {
        Analytics::AssociationRule rule = comparedRules.at(i);

        antecedent = this->analyst->extractEpisodeFromItemset(rule.antecedent);
        QString episode = antecedent.first.section(':', 1);
        QStandardItem * episodeItem = new QStandardItem(episode);
        episodeItem->setData(episode.toUpper(), Qt::UserRole);
        model->setItem(row, 0, episodeItem);

        QString circumstances = ((QStringList) antecedent.second).join(", ");
        QStandardItem * circumstancesItem = new QStandardItem(circumstances);
        circumstancesItem->setData(circumstances, Qt::UserRole);
        model->setItem(row, 1, circumstancesItem);

        QStandardItem * confidenceItem = new QStandardItem(QString("%1%").arg(QString::number(rule.confidence * 100, 'f', 2)));
        confidenceItem->setData(rule.confidence, Qt::UserRole);
        model->setItem(row, 2, confidenceItem);

        QChar plusOrEmpty = (confidenceVariance[i] > 0) ? QChar('+') : QChar(QChar::Null);
        QStandardItem * confidenceVarianceItem = new QStandardItem(QString("%1%2%").arg(plusOrEmpty).arg(QString::number(confidenceVariance[i] * 100, 'f', 2)));
        confidenceItem->setData(confidenceVariance[i], Qt::UserRole);
        model->setItem(row, 3, confidenceVarianceItem);

        Analytics::SupportCount eventCount;
        if (supportVariance[i] == -1) // This only existed in the "older" time range.
            eventCount = eventsInOlderTimeRange;
        else if (supportVariance[i] == 1) // This only existed in the "newer" time range.
            eventCount = eventsInNewerTimeRange;
        else // This existed in both time ranges.
            eventCount = eventsInIntersectedTimeRange;
        double relOccurrences = rule.support * 100.0 / eventCount;
        QStandardItem * occurrencesItem = new QStandardItem(
                    QString("%1 (%2%)")
                    .arg(QString::number(rule.support))
                    .arg(QString::number(relOccurrences, 'f', 2))
        );
        occurrencesItem->setData(rule.support, Qt::UserRole);
        model->setItem(row, 4, occurrencesItem);

        QChar plusOrEmpty2 = (supportVariance[i] > 0) ? QChar('+') : QChar(QChar::Null);
        QStandardItem * supportVarianceItem = new QStandardItem(QString("%1%2%").arg(plusOrEmpty2).arg((QString::number(supportVariance[i] * 100, 'f', 2))));
        supportVarianceItem->setData(supportVariance[i], Qt::UserRole);
        model->setItem(row, 5, supportVarianceItem);

        row++;
    }
    model->setSortRole(Qt::UserRole);

    this->causesTableProxyModel->setSourceModel(model);
    this->causesTable->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
}



//------------------------------------------------------------------------------
// Protected slots: UI-only.

void MainWindow::causesActionChanged(int action) {
    if (action == 0)
        this->updateCausesComparisonAbility(false); // Mine.
    else
        this->updateCausesComparisonAbility(true); // Compare.

    this->mineOrCompare();
}

void MainWindow::causesTimerangeChanged() {
    this->mineOrCompare();
}

void MainWindow::causesFilterChanged(QString filterString) {
    this->causesTableProxyModel->invalidate();

    QString episodeFilter = QString::null;
    QStringList circumstancesFilter;
    foreach (QString f, filterString.split(",", QString::SkipEmptyParts)) {
        f = f.trimmed();
        if (f.startsWith("episode:"))
            episodeFilter = f.section(':', 1);
        else
            circumstancesFilter.append(f);
    }

    this->causesTableProxyModel->setEpisodeFilter(episodeFilter);
    this->causesTableProxyModel->setCircumstancesFilter(circumstancesFilter);
}

void MainWindow::importFile() {
    static QString lastDirectory = "~/Desktop";
    QString logFile = QFileDialog::getOpenFileName(this, tr("Open Episodes log file"), lastDirectory, tr("Episodes log files (*.log)"), NULL, QFileDialog::ReadOnly);

    if (!logFile.isEmpty()) {
        lastDirectory = QFileInfo(logFile).path();
        emit parse(logFile);
    }
}


//------------------------------------------------------------------------------
// Private methods: logic.

void MainWindow::initLogic() {
    qRegisterMetaType< QList<QStringList> >("QList<QStringList>");
    qRegisterMetaType< QList<float> >("QList<float>");
    qRegisterMetaType<Time>("Time");
    Analytics::registerBasicMetaTypes();

    EpisodesParser::Parser::initParserHelpers("./config/browscap.csv",
                                              "./config/browscap-index.db",
                                              "./config/GeoIPCity.dat",
                                              "./config/GeoIPASNum.dat",
                                              "./config/EpisodesSpeeds.csv"
                                              );

    // Instantiate the EpisodesParser and the Analytics. Then connect them.
    this->parser = new EpisodesParser::Parser();

    // TODO: when these parameters have been figured out, they should be the defaults
    // and therefor they should be moved to the Analyst constructor.
    this->analyst = new Analytics::Analyst(0.05, 0.04, 0.2);

    // Set constraints. This defines which associations will be found. By
    // default, only causes for slow episodes will be searched.
    this->analyst->addFrequentItemsetItemConstraint("episode:*", Analytics::CONSTRAINT_POSITIVE_MATCH_ANY);
    this->analyst->addRuleConsequentItemConstraint("duration:slow", Analytics::CONSTRAINT_POSITIVE_MATCH_ANY);
    //analyst->addRuleConsequentItemConstraint("duration:acceptable", Analytics::CONSTRAINT_POSITIVE_MATCH_ANY);
    //analyst->addRuleConsequentItemConstraint("duration:fast", Analytics::CONSTRAINT_POSITIVE_MATCH_ANY);
}

void MainWindow::connectLogic() {
    // Pure logic.
    connect(this->parser, SIGNAL(parsedBatch(QList<QStringList>, double, Time, Time)), this->analyst, SLOT(analyzeTransactions(QList<QStringList>, double, Time, Time)));

    // Logic -> main thread -> logic (wake up sleeping threads).
    connect(this->analyst, SIGNAL(processedBatch()), SLOT(wakeParser()));

    // Logic -> UI.
    connect(this->parser, SIGNAL(parsing(bool)), SLOT(updateParsingStatus(bool)));
    connect(this->parser, SIGNAL(parsedDuration(int)), SLOT(updateParsingDuration(int)));
    connect(this->analyst, SIGNAL(analyzing(bool,Time,Time,int,int)), SLOT(updateAnalyzingStatus(bool,Time,Time,int,int)));
    connect(this->analyst, SIGNAL(analyzedDuration(int)), SLOT(updateAnalyzingDuration(int)));
    connect(this->analyst, SIGNAL(mining(bool)), SLOT(updateMiningStatus(bool)));
    connect(this->analyst, SIGNAL(minedDuration(int)), SLOT(updateMiningDuration(int)));
    connect(this->analyst, SIGNAL(stats(Time,Time,int,int,int,int,int)), SLOT(updateAnalyzingStats(Time,Time,int,int,int,int,int)));
    connect(this->analyst, SIGNAL(minedRules(uint,uint,QList<Analytics::AssociationRule>,Analytics::SupportCount)), SLOT(minedRules(uint,uint,QList<Analytics::AssociationRule>,Analytics::SupportCount)));
    connect(
                this->analyst,
                SIGNAL(comparedMinedRules(uint,uint,uint,uint,QList<Analytics::AssociationRule>,QList<Analytics::AssociationRule>,QList<Analytics::AssociationRule>,QList<Analytics::AssociationRule>,QList<Analytics::Confidence>,QList<float>,Analytics::SupportCount,Analytics::SupportCount,Analytics::SupportCount)),
                SLOT(comparedMinedRules(uint,uint,uint,uint,QList<Analytics::AssociationRule>,QList<Analytics::AssociationRule>,QList<Analytics::AssociationRule>,QList<Analytics::AssociationRule>,QList<Analytics::Confidence>,QList<float>,Analytics::SupportCount,Analytics::SupportCount,Analytics::SupportCount))
    );

    // UI -> logic.
    connect(this, SIGNAL(parse(QString)), this->parser, SLOT(parse(QString)));
    connect(this, SIGNAL(mine(uint,uint)), this->analyst, SLOT(mineRules(uint,uint)));
    connect(this, SIGNAL(mineAndCompare(uint,uint,uint,uint)), this->analyst, SLOT(mineAndCompareRules(uint,uint,uint,uint)));
}

void MainWindow::assignLogicToThreads() {
    this->parser->moveToThread(&this->parserThread);
    this->analyst->moveToThread(&this->analystThread);

    this->parserThread.start();
    this->analystThread.start();
}


//------------------------------------------------------------------------------
// Private methods: UI updating.

void MainWindow::updateStatus(const QString & status) {
    if (!status.isNull())
        this->statusCurrentlyProcessing->setText(status);
    else {
        QMutexLocker(&this->statusMutex);
        if (this->parsing)
            this->statusCurrentlyProcessing->setText(tr("Parsing"));
        else
            this->statusCurrentlyProcessing->setText(tr("Idle"));
    }
}

void MainWindow::updateCausesComparisonAbility(bool able) {
    if (!able) {
        this->causesCompareLabel->hide();
        this->causesCompareTimerangeChoice->hide();
    }
    else {
        this->causesCompareLabel->show();
        this->causesCompareTimerangeChoice->show();
    }
}

void MainWindow::mineOrCompare() {
    if (this->causesActionChoice->currentIndex() == 0) {
        QPair<uint, uint> buckets = MainWindow::mapTimerangeChoiceToBucket(this->causesMineTimerangeChoice->currentIndex());
        emit mine(buckets.first, buckets.second);
    }
    else {
        QPair<uint, uint> older = MainWindow::mapTimerangeChoiceToBucket(this->causesMineTimerangeChoice->currentIndex());
        QPair<uint, uint> newer = MainWindow::mapTimerangeChoiceToBucket(this->causesCompareTimerangeChoice->currentIndex());

        // Don't compare identical time ranges.
        if (older.first != newer.first || older.second != newer.second)
            emit mineAndCompare(older.first, older.second, newer.first, newer.second);
    }
}

QPair<uint, uint> MainWindow::mapTimerangeChoiceToBucket(int choice) {
    uint from, to;

    switch (choice) {
    case 0: // last quarter
        from = to = 0;
        break;
    case 1: // last hour
        from = to = 4;
        break;
    case 2: // last day
        from = to = 28;
        break;
    case 3: // last week
        from = 28; to = 34;
        break;
    case 4: // last month
        from = to = 59;
        break;
    case 5: // last year
        from = to = 71;
        break;
    case 6:
    default: // entire data set
        from = 0; to = 71;
        break;
    }

    return qMakePair(from, to);
}


//------------------------------------------------------------------------------
// Private methods: UI set-up.

void MainWindow::initUI() {
//    this->createSparklineGroupbox();
    this->createStatsGroupbox();
    this->createCausesGroupbox();
    this->createStatusGroupbox();
    this->createMenuBar();

    this->mainLayout = new QVBoxLayout();
//    this->mainLayout->addWidget(this->sparklineGroupbox);
//    this->mainLayout->addWidget(this->statsGroupbox);
    this->mainLayout->addWidget(this->causesGroupbox);
    this->mainLayout->addWidget(this->statusGroupbox);

    QWidget * widget = new QWidget();
    widget->setLayout(this->mainLayout);
    this->setCentralWidget(widget);

    this->setWindowTitle(tr("WPO Analytics"));
    this->resize(790, 580);
}

void MainWindow::createSparklineGroupbox() {
    this->sparklineGroupbox = new QGroupBox(tr("Sparkline"));
    QVBoxLayout * layout = new QVBoxLayout();

    // Add children to layout.
    this->label = new QLabel("test");
    layout->addWidget(this->label);

    // Set layout for groupbox.
    this->sparklineGroupbox->setLayout(layout);
}

void MainWindow::createStatsGroupbox() {
    this->statsGroupbox = new QGroupBox(tr("Statistics"));
    QVBoxLayout * layout = new QVBoxLayout();

    // Add children to layout.
    QHBoxLayout * barLayout = new QHBoxLayout();
    QVBoxLayout * statsLayout = new QVBoxLayout();
    layout->addLayout(barLayout);
    layout->addLayout(statsLayout);

    // Add children to bar layout.
    QLabel * bl1 = new QLabel(tr("Average"));
    this->statsEpisodeComboBox = new QComboBox();
    this->statsEpisodeComboBox->addItems(QStringList() << "css" << "js" << "pageready");
    QLabel * bl2 = new QLabel(tr("load time for"));
    this->statsLocationComboBox = new QComboBox();
    this->statsLocationComboBox->addItems(QStringList() << "Belgium" << "France");
    QLabel * bl3 = new QLabel(tr("in the last:"));
    barLayout->addWidget(bl1);
    barLayout->addWidget(this->statsEpisodeComboBox);
    barLayout->addWidget(bl2);
    barLayout->addWidget(this->statsLocationComboBox);
    barLayout->addWidget(bl3);


    // Add children to stats layout.
    QHBoxLayout * labelLayout = new QHBoxLayout();
    QHBoxLayout * dataLayout = new QHBoxLayout();
    layout->addLayout(labelLayout);
    layout->addLayout(dataLayout);
    QLabel * cl1 = new QLabel("year");
    QLabel * cl2 = new QLabel("month");
    QLabel * cl3 = new QLabel("week");
    QLabel * cl4 = new QLabel("day");
    QLabel * cl5 = new QLabel("hour");
    QLabel * cl6 = new QLabel("quarter");
    labelLayout->addWidget(cl1);
    labelLayout->addWidget(cl2);
    labelLayout->addWidget(cl3);
    labelLayout->addWidget(cl4);
    labelLayout->addWidget(cl5);
    labelLayout->addWidget(cl6);

    // Add sample data.
    QLabel * scl1 = new QLabel("2047 ms");
    QLabel * scl2 = new QLabel("1903 ms");
    QLabel * scl3 = new QLabel("1809 ms");
    QLabel * scl4 = new QLabel("1857 ms");
    QLabel * scl5 = new QLabel("1634 ms");
    QLabel * scl6 = new QLabel("1698 ms");
    dataLayout->addWidget(scl1);
    dataLayout->addWidget(scl2);
    dataLayout->addWidget(scl3);
    dataLayout->addWidget(scl4);
    dataLayout->addWidget(scl5);
    dataLayout->addWidget(scl6);


    // Set layout for groupbox.
    this->statsGroupbox->setLayout(layout);
}

void MainWindow::createCausesGroupbox() {
    this->causesGroupbox = new QGroupBox(tr("Causes"));
    QVBoxLayout * layout = new QVBoxLayout();
    QHBoxLayout * mineLayout = new QHBoxLayout();
    QHBoxLayout * filterLayout = new QHBoxLayout();
    QHBoxLayout * descriptionLayout = new QHBoxLayout();

    // Add children to layout.
    layout->addLayout(mineLayout);
    layout->addLayout(descriptionLayout);
    this->causesTable = new QTableView(this);
    this->causesTableProxyModel = new CausesTableFilterProxyModel(this);
    this->causesTableProxyModel->setSortRole(Qt::UserRole);
    this->causesTableProxyModel->setFilterRole(Qt::DisplayRole);
    this->causesTableProxyModel->setEpisodesColumn(0);
    this->causesTableProxyModel->setCircumstancesColumn(1);
    this->causesTable->setModel(this->causesTableProxyModel);
    this->causesTable->setSortingEnabled(true);
    this->causesTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(this->causesTable);
    layout->addLayout(filterLayout);

    // Add children to "mine" layout.
    QStringList timeRanges = QStringList() << "last quarter"
                                           << "last hour"
                                           << "last day"
                                           << "last week"
                                           << "last month"
                                           << "last year"
                                           << "entire data set";
    this->causesActionChoice = new QComboBox(this);
    this->causesActionChoice->addItem(tr("Mine"));
    this->causesActionChoice->addItem(tr("Compare"));
    QLabel * cm1 = new QLabel(tr("causes in the"));
    this->causesMineTimerangeChoice = new QComboBox(this);
    this->causesMineTimerangeChoice->addItems(timeRanges);
    this->causesCompareLabel = new QLabel(tr("with those in the"));
    this->causesCompareTimerangeChoice = new QComboBox(this);
    this->causesCompareTimerangeChoice->addItems(timeRanges);
    mineLayout->addWidget(this->causesActionChoice);
    mineLayout->addWidget(cm1);
    mineLayout->addWidget(this->causesMineTimerangeChoice);
    mineLayout->addWidget(this->causesCompareLabel);
    mineLayout->addWidget(this->causesCompareTimerangeChoice);
    mineLayout->addStretch();
    this->updateCausesComparisonAbility(false);

    // Add children to "filter" layout.
    QLabel * filterLabel = new QLabel(tr("Filter") + ":");
    this->causesFilterCompleter = new ConceptHierarchyCompleter();
    this->causesFilterCompleter->setCaseSensitivity(Qt::CaseInsensitive);
    this->causesFilterCompleter->setModel(this->analyst->getConceptHierarchyModel());
    this->causesFilterCompleter->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
    this->causesFilter = new QLineEdit();
    this->causesFilter->setCompleter(this->causesFilterCompleter);
    filterLayout->addWidget(filterLabel);
    filterLayout->addWidget(this->causesFilter);

    // Add children to "description" layout.
    this->causesDescription = new QLabel(tr("No causes have been mined yet."));
    descriptionLayout->addWidget(this->causesDescription);
    descriptionLayout->addStretch();

    // Set layout for groupbox.
    this->causesGroupbox->setLayout(layout);
}

void MainWindow::createStatusGroupbox() {
    this->statusGroupbox = new QGroupBox(tr("Status"));
    QVBoxLayout * layout = new QVBoxLayout();

    // Add children to layout.
    QHBoxLayout * currentlyProcessingLayout = new QHBoxLayout();
    QGroupBox * measurementsGroupbox = new QGroupBox(tr("Measurements statistics"));
    QGroupBox * miningGroupbox = new QGroupBox(tr("Mining statistics"));
    QGroupBox * performanceGroupbox = new QGroupBox(tr("Performance"));
    layout->addLayout(currentlyProcessingLayout);
    layout->addWidget(measurementsGroupbox);
    layout->addWidget(miningGroupbox);
    layout->addWidget(performanceGroupbox);

    // Add children to "currently processing" layout.
    this->statusCurrentlyProcessing = new QLabel(tr("Idle"));
    currentlyProcessingLayout->addWidget(this->statusCurrentlyProcessing);
    currentlyProcessingLayout->addStretch();

    // Add children to "measurements" groupbox.
    QHBoxLayout * measurementsLayout = new QHBoxLayout();
    QLabel * me1 = new QLabel(tr("Start date:"));
    this->status_measurements_startDate = new QLabel(tr("N/A yet"));
    QLabel * me2 = new QLabel(tr("End date:"));
    this->status_measurements_endDate = new QLabel(tr("N/A yet"));
    QLabel * me3 = new QLabel(tr("Page views:"));
    this->status_measurements_pageViews = new QLabel("0");
    QLabel * me4 = new QLabel(tr("Episodes:"));
    this->status_measurements_episodes = new QLabel("0");
    measurementsLayout->addWidget(me1);
    measurementsLayout->addWidget(this->status_measurements_startDate);
    measurementsLayout->addStretch();
    measurementsLayout->addWidget(me2);
    measurementsLayout->addWidget(this->status_measurements_endDate);
    measurementsLayout->addStretch();
    measurementsLayout->addWidget(me3);
    measurementsLayout->addWidget(this->status_measurements_pageViews);
    measurementsLayout->addStretch();
    measurementsLayout->addWidget(me4);
    measurementsLayout->addWidget(this->status_measurements_episodes);
    measurementsGroupbox->setLayout(measurementsLayout);

    // Add children to "mining" groupbox.
    QHBoxLayout * miningLayout = new QHBoxLayout();
    QLabel * mir1_1 = new QLabel(tr("Unique items:"));
    this->status_mining_uniqueItems = new QLabel("0");
    QLabel * mir1_2 = new QLabel(tr("Frequent items:"));
    this->status_mining_frequentItems = new QLabel("0");
    QLabel * mir1_3 = new QLabel(tr("Pattern Tree:"));
    this->status_mining_patternTree = new QLabel("0");
    miningLayout->addWidget(mir1_1);
    miningLayout->addWidget(this->status_mining_uniqueItems);
    miningLayout->addStretch();
    miningLayout->addWidget(mir1_2);
    miningLayout->addWidget(this->status_mining_frequentItems);
    miningLayout->addStretch();
    miningLayout->addWidget(mir1_3);
    miningLayout->addWidget(this->status_mining_patternTree);
    miningGroupbox->setLayout(miningLayout);

    // Add children to "performance" groupbox.
    QHBoxLayout * performanceLayout = new QHBoxLayout();
    QLabel * mir2_1 = new QLabel(tr("Parsing:"));
    this->status_performance_parsing = new QLabel("0 s");
    QLabel * mir2_2 = new QLabel(tr("Analyzing:"));
    this->status_performance_analyzing = new QLabel("0 s");
    QLabel * mir2_3 = new QLabel(tr("Mining:"));
    this->status_performance_mining = new QLabel("0 s");
    performanceLayout->addWidget(mir2_1);
    performanceLayout->addWidget(this->status_performance_parsing);
    performanceLayout->addStretch();
    performanceLayout->addWidget(mir2_2);
    performanceLayout->addWidget(this->status_performance_analyzing);
    performanceLayout->addStretch();
    performanceLayout->addWidget(mir2_3);
    performanceLayout->addWidget(this->status_performance_mining);
    performanceGroupbox->setLayout(performanceLayout);

    // Set layout for groupbox.
    this->statusGroupbox->setLayout(layout);
}

void MainWindow::createMenuBar() {
    QMenuBar * menuBar = this->menuBar();

    this->menuFile = new QMenu(tr("File"), menuBar);
    this->menuFileImport = new QAction(tr("Import"), this->menuFile);
    this->menuFileImport->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_I));
    this->menuFile->addAction(this->menuFileImport);

    menuBar->addMenu(this->menuFile);
}

void MainWindow::connectUI() {
    connect(this->causesActionChoice, SIGNAL(currentIndexChanged(int)), SLOT(causesActionChanged(int)));
    connect(this->causesMineTimerangeChoice, SIGNAL(currentIndexChanged(int)), SLOT(causesTimerangeChanged()));
    connect(this->causesCompareTimerangeChoice, SIGNAL(currentIndexChanged(int)), SLOT(causesTimerangeChanged()));
    connect(this->causesFilter, SIGNAL(textChanged(QString)), SLOT(causesFilterChanged(QString)));

    // Menus.
    connect(this->menuFileImport, SIGNAL(triggered()), SLOT(importFile()));
}
