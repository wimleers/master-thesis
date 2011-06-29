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
#include <QSettings>

class SettingsDialog : public QDialog {

    Q_OBJECT

public:
    explicit SettingsDialog(QWidget * parent = NULL);

    static double errorMarginToAbsolute(double minSupport, double errorMargin);
    static double absoluteToErrorMargin(double minSupport, double minPatternTreeSupport);

signals:

public slots:

protected slots:
    void minSupportChanged(double value);
    void minConfidenceChanged(double value);
    void patternTreeSupportErrorMarginChanged(double value);

    void buttonRestoreDefaults();
    void buttonCancel();
    void buttonSave();

protected:
    QDoubleSpinBox * minSupport;
    QDoubleSpinBox * minConfidence;
    QDoubleSpinBox * patternTreeSupportErrorMargin;
    QLabel * resultingParametersMinSupport;
    QLabel * resultingParametersMinConfidence;
    QLabel * resultingParametersMinPatternTreeSupport;
};

#endif // SETTINGSDIALOG_H
