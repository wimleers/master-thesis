#include "SettingsDialog.h"

SettingsDialog::SettingsDialog(QWidget * parent) :
    QDialog(parent)
{
    QTabWidget * mainWidget = new QTabWidget(this);

    mainWidget->addTab(this->createAnalystTab(), tr("Analysis"));
    mainWidget->addTab(this->createParserTab(), tr("Parser"));

    // Buttons.
    QDialogButtonBox * buttonBox = new QDialogButtonBox(this);
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->addButton(QDialogButtonBox::RestoreDefaults);
    buttonBox->addButton(QDialogButtonBox::Cancel);
    buttonBox->addButton(QDialogButtonBox::Save);
    buttonBox->button(QDialogButtonBox::RestoreDefaults)->setAutoDefault(false);

    // Layout.
    QVBoxLayout * mainLayout = new QVBoxLayout();
    this->setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    mainLayout->addWidget(buttonBox);

    // Connections.
    connect(this->minSupport, SIGNAL(valueChanged(double)), SLOT(minSupportChanged(double)));
    connect(this->minConfidence, SIGNAL(valueChanged(double)), SLOT(minConfidenceChanged(double)));
    connect(this->patternTreeSupportErrorMargin, SIGNAL(valueChanged(double)), SLOT(patternTreeSupportErrorMarginChanged(double)));
    connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), SIGNAL(clicked()), SLOT(buttonRestoreDefaults()));
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), SLOT(buttonCancel()));
    connect(buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), SLOT(buttonSave()));
}

double SettingsDialog::errorMarginToAbsolute(double minSupport, double errorMargin) {
    double sigma = minSupport / 100.0;
    double e = errorMargin / 100.0;
    return sigma * (1 - e) * 100.0;
}

double SettingsDialog::absoluteToErrorMargin(double minSupport, double minPatternTreeSupport) {
    double sigma = minSupport / 100.0;
    double epsilon = minPatternTreeSupport / 100.0;
    return (1.0 - (epsilon / sigma)) * 100.0;
}

void SettingsDialog::minSupportChanged(double value) {
    this->resultingParametersMinSupport->setText(QString("%1%").arg(QString::number(value, 'f', 2)));
    this->resultingParametersMinPatternTreeSupport->setText(QString("%1%").arg(QString::number(SettingsDialog::errorMarginToAbsolute(value, this->patternTreeSupportErrorMargin->value()), 'f', 2)));
}

void SettingsDialog::minConfidenceChanged(double value) {
    this->resultingParametersMinConfidence->setText(QString("%1%").arg(QString::number(value, 'f', 2)));
}

void SettingsDialog::patternTreeSupportErrorMarginChanged(double value) {
    this->resultingParametersMinPatternTreeSupport->setText(QString("%1%").arg(QString::number(SettingsDialog::errorMarginToAbsolute(this->minSupport->value(), value), 'f', 2)));
}

void SettingsDialog::browseForFile() {
    QString episodesDiscretizerCSVFile = QFileDialog::getOpenFileName(this, tr("Open Episodes Discretizer settings file"), "", tr("Episodes Discretizer settings file (*.csv)"), NULL, QFileDialog::ReadOnly);
    if (!episodesDiscretizerCSVFile.isEmpty()) {
        this->parserEpisodesDiscretizerFileLineEdit->setText(episodesDiscretizerCSVFile);
    }
}

void SettingsDialog::buttonRestoreDefaults() {
    QSettings settings;
    settings.remove("analyst");
    settings.remove("parser");

    this->close();

    emit this->settingsChanged();
}

void SettingsDialog::buttonCancel() {
    this->close();
}

void SettingsDialog::buttonSave() {
    QSettings settings;

    settings.beginGroup("analyst");
    settings.setValue("minimumSupport", this->minSupport->value() / 100.0);
    settings.setValue("minimumPatternTreeSupport", SettingsDialog::errorMarginToAbsolute(this->minSupport->value(), this->patternTreeSupportErrorMargin->value()) / 100.0);
    settings.setValue("minimumConfidence", this->minConfidence->value() / 100.0);
    settings.endGroup();

    settings.beginGroup("parser");
    settings.setValue("episodeDiscretizerCSVFile", this->parserEpisodesDiscretizerFileLineEdit->text());
    settings.endGroup();

    this->close();

    emit this->settingsChanged();
}

QWidget * SettingsDialog::createAnalystTab() {
    QSettings settings;

    QWidget * tab = new QWidget();

    QLabel * descriptionLabel = new QLabel(tr("Find causes for all problems that:"));

    // Minimum frequent itemset support.
    QHBoxLayout * minSupLayout = new QHBoxLayout();
    QLabel * minSupPrefixLabel = new QLabel(tr("affect at least"));
    QLabel * minSupSuffixLabel = new QLabel(tr("of all page views"));
    this->minSupport = new QDoubleSpinBox();
    this->minSupport->setSuffix("%");
    this->minSupport->setMinimum(1);
    this->minSupport->setMaximum(100);
    this->minSupport->setValue(qRound(settings.value("analyst/minimumSupport", 0.05).toDouble() * 100));
    minSupLayout->addWidget(minSupPrefixLabel);
    minSupLayout->addWidget(this->minSupport);
    minSupLayout->addWidget(minSupSuffixLabel);
    minSupLayout->addStretch();

    // Minimum PatternTree support.
    QHBoxLayout * minPatternTreeSupLayout = new QHBoxLayout();
    QLabel * minPatternTreeSupPrefixLabel = new QLabel(tr("(but allow a frequency estimation error margin of"));
    QLabel * minPatternTreeSupSuffixLabel = new QLabel(tr(")"));
    this->patternTreeSupportErrorMargin = new QDoubleSpinBox();
    this->patternTreeSupportErrorMargin->setSuffix("%");
    this->patternTreeSupportErrorMargin->setMinimum(1);
    this->patternTreeSupportErrorMargin->setMaximum(100);
    this->patternTreeSupportErrorMargin->setValue(SettingsDialog::absoluteToErrorMargin(this->minSupport->value(), settings.value("analyst/minimumPatternTreeSupport", 0.04).toDouble() * 100));

    minPatternTreeSupLayout->addWidget(minPatternTreeSupPrefixLabel);
    minPatternTreeSupLayout->addWidget(this->patternTreeSupportErrorMargin);
    minPatternTreeSupLayout->addWidget(minPatternTreeSupSuffixLabel);
    minPatternTreeSupLayout->addStretch();

    // Minimum confidence.
    QHBoxLayout * minConfidenceLayout = new QHBoxLayout();
    QLabel * minConfidencePrefixLabel = new QLabel(tr("and are slow at least"));
    QLabel * minConfidenceSuffixLabel = new QLabel(tr("of the time"));
    this->minConfidence = new QDoubleSpinBox();
    this->minConfidence->setSuffix("%");
    this->minConfidence->setMinimum(1);
    this->minConfidence->setMaximum(100);
    this->minConfidence->setValue(qRound(settings.value("analyst/minimumConfidence", 0.2).toDouble() * 100));
    minConfidenceLayout->addWidget(minConfidencePrefixLabel);
    minConfidenceLayout->addWidget(this->minConfidence);
    minConfidenceLayout->addWidget(minConfidenceSuffixLabel);
    minConfidenceLayout->addStretch();

    // Resulting parameters.
    QGroupBox * resultingParameters = new QGroupBox(tr("Resulting parameters"));
    QVBoxLayout * resultingParametersLayout = new QVBoxLayout();
    resultingParameters->setLayout(resultingParametersLayout);

    QHBoxLayout * rpMinSupportLayout = new QHBoxLayout();
    this->resultingParametersMinSupport = new QLabel(QString("%1%").arg(QString::number(this->minSupport->value(), 'f', 2)));
    rpMinSupportLayout->addWidget(new QLabel(tr("Minimum frequent itemset support") + ":"));
    rpMinSupportLayout->addWidget(this->resultingParametersMinSupport);
    rpMinSupportLayout->addStretch();
    resultingParametersLayout->addLayout(rpMinSupportLayout);

    QHBoxLayout * rpMinPatternTreeSupportLayout = new QHBoxLayout();
    this->resultingParametersMinPatternTreeSupport = new QLabel(QString("%1%").arg(QString::number(SettingsDialog::errorMarginToAbsolute(this->minSupport->value(), this->patternTreeSupportErrorMargin->value()), 'f', 2)));
    rpMinPatternTreeSupportLayout->addWidget(new QLabel(tr("Minimum PatternTree suppport") + ":"));
    rpMinPatternTreeSupportLayout->addWidget(this->resultingParametersMinPatternTreeSupport);
    rpMinPatternTreeSupportLayout->addStretch();
    resultingParametersLayout->addLayout(rpMinPatternTreeSupportLayout);

    QHBoxLayout * rpMinConfidenceLayout = new QHBoxLayout();
    this->resultingParametersMinConfidence = new QLabel(QString("%1%").arg(QString::number(this->minConfidence->value(), 'f', 2)));
    rpMinConfidenceLayout->addWidget(new QLabel(tr("Minimum confidence") + ":"));
    rpMinConfidenceLayout->addWidget(this->resultingParametersMinConfidence);
    rpMinConfidenceLayout->addStretch();
    resultingParametersLayout->addLayout(rpMinConfidenceLayout);

    // Layout.
    QVBoxLayout * mainLayout = new QVBoxLayout();
    tab->setLayout(mainLayout);
    mainLayout->addWidget(descriptionLabel);
    mainLayout->addLayout(minSupLayout);
    mainLayout->addLayout(minPatternTreeSupLayout);
    mainLayout->addLayout(minConfidenceLayout);
    mainLayout->addWidget(resultingParameters);

    return tab;
}

QWidget * SettingsDialog::createParserTab() {
    QSettings settings;

    QWidget * tab = new QWidget();

    QHBoxLayout * mainLayout = new QHBoxLayout();

    mainLayout->addWidget(new QLabel(tr("Episodes discretizer settings")));
    QString basePath = QCoreApplication::applicationDirPath();
    QString defaultValue = basePath + "/config/EpisodesSpeeds.csv";
    this->parserEpisodesDiscretizerFileLineEdit = new QLineEdit(settings.value("parser/episodeDiscretizerCSVFile", defaultValue).toString());
    mainLayout->addWidget(this->parserEpisodesDiscretizerFileLineEdit);
    QPushButton * browseButton = new QPushButton(tr("Browse"));
    browseButton->setAutoDefault(false);
    mainLayout->addWidget(browseButton);

    connect(browseButton, SIGNAL(pressed()), SLOT(browseForFile()));

    tab->setLayout(mainLayout);

    return tab;
}
