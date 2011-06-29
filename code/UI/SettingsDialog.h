#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QGroupBox>
#include <QTableWidget>
#include <QLineEdit>
#include <QFileDialog>
#include <QSettings>
#include <QApplication>

class SettingsDialog : public QDialog {

    Q_OBJECT

public:
    explicit SettingsDialog(QWidget * parent = NULL);

    static double errorMarginToAbsolute(double minSupport, double errorMargin);
    static double absoluteToErrorMargin(double minSupport, double minPatternTreeSupport);

signals:
    void settingsChanged();

protected slots:
    void minSupportChanged(double value);
    void minConfidenceChanged(double value);
    void patternTreeSupportErrorMarginChanged(double value);

    void browseForFile();

    void buttonRestoreDefaults();
    void buttonCancel();
    void buttonSave();

    void restart();

protected:
    QWidget * createAnalystTab();
    QWidget * createParserTab();

    // Analyst settings tab.
    QDoubleSpinBox * minSupport;
    QDoubleSpinBox * minConfidence;
    QDoubleSpinBox * patternTreeSupportErrorMargin;
    QLabel * resultingParametersMinSupport;
    QLabel * resultingParametersMinConfidence;
    QLabel * resultingParametersMinPatternTreeSupport;

    // Parser settings tab.
    QLineEdit * parserEpisodesDiscretizerFileLineEdit;
};

#endif // SETTINGSDIALOG_H
