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

    // Temporary demo.
    QString file("/Users/wimleers/School/masterthesis/logs/driverpacks.net.episodes.first-chunk.log");
    emit parse(file);

    /*
    analyst->mineRules(0, (uint) TTW_NUM_BUCKETS - 1); // Rules over the entire dataset.
    analyst->mineRules(4, 4); // Rules over the past hour.
    analyst->mineAndCompareRules(0, 10, 4, 4);
    */
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
    this->updateMiningAbility(!this->parsing);
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
    Q_UNUSED(from);
    Q_UNUSED(to);

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

    QStandardItemModel * model = new QStandardItemModel(associationRules.size(), 5, this);

    QStringList headerLabels;
    headerLabels << tr("Episode") << tr("Circumstances") << tr("% slow") << tr("# slow") << tr("% of page views");
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

        QStandardItem * occurrencesItem = new QStandardItem(QString::number(rule.support));
        occurrencesItem->setData(rule.support, Qt::UserRole);
        model->setItem(row, 3, occurrencesItem);

        double relOccurrences = rule.support * 100.0 / eventsInTimeRange;
        QStandardItem * relOccurrencesItem = new QStandardItem(QString("%1%").arg(QString::number(relOccurrences, 'f', 2)));
        relOccurrencesItem->setData(relOccurrences, Qt::UserRole);
        model->setItem(row, 4, relOccurrencesItem);

        row++;
    }
    model->setSortRole(Qt::UserRole);

    this->causesTableProxyModel->setSourceModel(model);
    this->causesTable->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
}


//------------------------------------------------------------------------------
// Protected slots: UI-only.

void MainWindow::mineLastQuarter() {
    this->mineTimeRange(0, 0);
}

void MainWindow::mineLastHour() {
    this->mineTimeRange(4, 4);
}

void MainWindow::mineLastDay() {
    this->mineTimeRange(28, 28);
}

void MainWindow::mineLastWeek() {
    this->mineTimeRange(28, 34);
}

void MainWindow::mineLastMonth() {
    this->mineTimeRange(59, 59);
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


//------------------------------------------------------------------------------
// Protected slots: UI -> Analyst.

void MainWindow::mineAllTime() {
    this->mineTimeRange(0, 71);
}


//------------------------------------------------------------------------------
// Protected slots: Analyst -> UI.

void MainWindow::mineTimeRange(uint from, uint to) {
    emit mine(from, to);
}


//------------------------------------------------------------------------------
// Private methods: logic.

void MainWindow::initLogic() {
    qRegisterMetaType< QList<QStringList> >("QList<QStringList>");
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
    connect(this->analyst, SIGNAL(minedDuration(int)), SLOT(updateMiningDuration(int)));
    connect(this->analyst, SIGNAL(stats(Time,Time,int,int,int,int,int)), SLOT(updateAnalyzingStats(Time,Time,int,int,int,int,int)));
    connect(this->analyst, SIGNAL(minedRules(uint,uint,QList<Analytics::AssociationRule>,Analytics::SupportCount)), SLOT(minedRules(uint,uint,QList<Analytics::AssociationRule>,Analytics::SupportCount)));

    // UI -> logic.
    connect(this, SIGNAL(parse(QString)), this->parser, SLOT(parse(QString)));
    connect(this, SIGNAL(mine(uint,uint)), this->analyst, SLOT(mineRules(uint,uint)));
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

void MainWindow::updateMiningAbility(bool enabled) {
    this->causesMineLastQuarterButton->setEnabled(enabled);
    this->causesMineLastHourButton->setEnabled(enabled);
    this->causesMineLastDayButton->setEnabled(enabled);
    this->causesMineLastWeekButton->setEnabled(enabled);
    this->causesMineLastMonthButton->setEnabled(enabled);
    this->causesMineAllTimeButton->setEnabled(enabled);
}


//------------------------------------------------------------------------------
// Private methods: UI set-up.

void MainWindow::initUI() {
//    this->createSparklineGroupbox();
    this->createStatsGroupbox();
    this->createCausesGroupbox();
    this->createStatusGroupbox();

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
    this->causesMineLastQuarterButton = new QPushButton(tr("Mine last quarter"));
    this->causesMineLastHourButton    = new QPushButton(tr("Mine last hour"));
    this->causesMineLastDayButton     = new QPushButton(tr("Mine last day"));
    this->causesMineLastWeekButton    = new QPushButton(tr("Mine last week"));
    this->causesMineLastMonthButton   = new QPushButton(tr("Mine last month"));
    this->causesMineAllTimeButton     = new QPushButton(tr("Mine all time"));
    mineLayout->addWidget(this->causesMineLastQuarterButton);
    mineLayout->addStretch();
    mineLayout->addWidget(this->causesMineLastHourButton);
    mineLayout->addStretch();
    mineLayout->addWidget(this->causesMineLastDayButton);
    mineLayout->addStretch();
    mineLayout->addWidget(this->causesMineLastWeekButton);
    mineLayout->addStretch();
    mineLayout->addWidget(this->causesMineLastMonthButton);
    mineLayout->addStretch();
    mineLayout->addWidget(this->causesMineAllTimeButton);
    this->updateMiningAbility(false);

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

void MainWindow::connectUI() {
    connect(this->causesMineLastQuarterButton, SIGNAL(pressed()), SLOT(mineLastQuarter()));
    connect(this->causesMineLastHourButton, SIGNAL(pressed()), SLOT(mineLastHour()));
    connect(this->causesMineLastDayButton, SIGNAL(pressed()), SLOT(mineLastDay()));
    connect(this->causesMineLastWeekButton, SIGNAL(pressed()), SLOT(mineLastWeek()));
    connect(this->causesMineLastMonthButton, SIGNAL(pressed()), SLOT(mineLastMonth()));
    connect(this->causesMineAllTimeButton, SIGNAL(pressed()), SLOT(mineAllTime()));

    connect(this->causesFilter, SIGNAL(textChanged(QString)), SLOT(causesFilterChanged(QString)));
}
