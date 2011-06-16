#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QStringList>
#include <QPair>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>

#include <QMainWindow>
#include <QMenuBar>
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

#include <QTableView>
#include <QHeaderView>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QSortFilterProxyModel>

#include "ConceptHierarchyCompleter.h"
#include "CausesTableFilterProxyModel.h"

#include "../EpisodesParser/Parser.h"
#include "../Analytics/Analyst.h"


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

protected slots:
    // UI-only.
    void mineLastQuarter();
    void mineLastHour();
    void mineLastDay();
    void mineLastWeek();
    void mineLastMonth();
    void causesFilterChanged();

    // UI -> Analyst.
    void mineAllTime();

    // Analyst -> UI.
    void mineTimeRange(uint from, uint to);

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
    void connectUI();

    // UI updating.
    void updateStatus(const QString & status = QString::null);
    void updateMiningAbility(bool enabled);

    // Logic.
    EpisodesParser::Parser * parser;
    Analytics::Analyst * analyst;
    QThread parserThread;
    QThread analystThread;

    // Stats.
    QMutex statusMutex;
    bool parsing;
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
    QPushButton * causesMineLastQuarterButton;
    QPushButton * causesMineLastHourButton;
    QPushButton * causesMineLastDayButton;
    QPushButton * causesMineLastWeekButton;
    QPushButton * causesMineLastMonthButton;
    QPushButton * causesMineAllTimeButton;
    QLineEdit * causesFilter;
    ConceptHierarchyCompleter * causesFilterCompleter;
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
};

#endif // MAINWINDOW_H
