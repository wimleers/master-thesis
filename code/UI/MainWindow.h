#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QStringList>
#include <QPair>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QChar>
#include <QDebug>

#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QKeySequence>
#include <QStatusBar>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>
#include <QDesktopServices>
#include <QDialog>

#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSortFilterProxyModel>

#include <QFileInfo>
#include <QSettings>

#include "ConceptHierarchyCompleter.h"
#include "CausesTableFilterProxyModel.h"
#include "SettingsDialog.h"

#include "../EpisodesParser/Parser.h"
#include "../Analytics/Analyst.h"
#include "../Analytics/TiltedTimeWindow.h"


#define STATS_ITEM_ESTIMATED_AVG_BYTES 20 * 4
#define STATS_TILTED_TIME_WINDOW_BYTES 292
#define STATS_FPNODE_FIXED_OVERHEAD_BYTES 12
#define STATS_FPNODE_ESTIMATED_CHILDREN_AVG_BYTES 3 * 4


class MainWindow : public QMainWindow {

    Q_OBJECT

public:
    explicit MainWindow(QWidget * parent = 0);
    ~MainWindow();

signals:
    void parse(QString file);
    void mine(uint from, uint to);
    void mineAndCompare(uint fromOlder, uint toOlder, uint fromNewer, uint toNewer);

public slots:
    // Parser.
    void wakeParser();
    void updateParsingStatus(bool parsing);
    void updateParsingDuration(int duration);

    // Analyst: analyzing.
    void updateAnalyzingStatus(bool analyzing, Time start, Time end, int numPageViews, int numTransactions);
    void updateAnalyzingDuration(int duration);
    void updateAnalyzingStats(Time start, Time end, int pageViews, int transactions, int uniqueItems, int frequentItems, int patternTreeSize);

    // Analyst: mining.
    void updateMiningStatus(bool mining);
    void updateMiningDuration(int duration);
    void minedRules(uint from, uint to, QList<Analytics::AssociationRule> associationRules, Analytics::SupportCount eventsInTimeRange);
    void comparedMinedRules(uint fromOlder, uint toOlder,
                            uint fromNewer, uint toNewer,
                            QList<Analytics::AssociationRule> intersectedRules,
                            QList<Analytics::AssociationRule> olderRules,
                            QList<Analytics::AssociationRule> newerRules,
                            QList<Analytics::AssociationRule> comparedRules,
                            QList<Analytics::Confidence> confidenceVariance,
                            QList<float> supportVariance,
                            Analytics::SupportCount eventsInIntersectedTimeRange,
                            Analytics::SupportCount eventsInOlderTimeRange,
                            Analytics::SupportCount eventsInNewerTimeRange);

protected slots:
    void causesActionChanged(int action);
    void causesTimerangeChanged();
    void causesFilterChanged(QString filterString);

    void importFile();
    void settingsDialog();

private:
    // Logic.
    void initLogic();
    void connectLogic();
    void assignLogicToThreads();

    // UI set-up.
    void initUI();
    void createSparklineGroupbox();
    void createStatsGroupbox();
    void createCausesGroupbox();
    void createStatusGroupbox();
    void createMenuBar();
    void connectUI();

    // UI updating.
    void updateStatus(const QString & status = QString::null);
    void updateCausesComparisonAbility(bool able);
    void mineOrCompare();
    static QPair<uint, uint> mapTimerangeChoiceToBucket(int choice);

    // Logic.
    EpisodesParser::Parser * parser;
    Analytics::Analyst * analyst;
    QThread parserThread;
    QThread analystThread;

    // Stats.
    QMutex statusMutex;
    bool parsing;
    int patternTreeSize;
    Time startTime;
    Time endTime;
    int totalPageViews;
    int totalTransactions;
    int totalPatternsExaminedWhileMining;
    int totalParsingDuration;
    int totalAnalyzingDuration;
    int totalMiningDuration;

    // Major widgets.
    QVBoxLayout * mainLayout;

    // Sparkline groupbox.
    QGroupBox * sparklineGroupbox;
    QLabel * label;

    // Stats groupbox.
    QGroupBox * statsGroupbox;
    QComboBox * statsEpisodeComboBox;
    QComboBox * statsLocationComboBox;

    // Causes groupbox.
    QGroupBox * causesGroupbox;
    QComboBox * causesActionChoice;
    QComboBox * causesMineTimerangeChoice;
    QLabel * causesCompareLabel;
    QComboBox * causesCompareTimerangeChoice;
    QLineEdit * causesFilter;
    ConceptHierarchyCompleter * causesFilterCompleter;
    QLabel * causesDescription;
    QTableView * causesTable;
    QStandardItemModel * causesTableModel;
    CausesTableFilterProxyModel * causesTableProxyModel;

    // Status groupbox.
    QGroupBox * statusGroupbox;
    QLabel * statusCurrentlyProcessing;
    QLabel * status_measurements_startDate;
    QLabel * status_measurements_endDate;
    QLabel * status_measurements_pageViews;
    QLabel * status_measurements_episodes;
    QLabel * status_performance_parsing;
    QLabel * status_performance_analyzing;
    QLabel * status_performance_mining;
    QLabel * status_mining_uniqueItems;
    QLabel * status_mining_frequentItems;
    QLabel * status_mining_patternTree;

    // Menu bar.
    QMenu * menuFile;
    QAction * menuFileImport;
    QAction * menuFileSettings;
};

#endif // MAINWINDOW_H
