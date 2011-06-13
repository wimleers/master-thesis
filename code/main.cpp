#include "QCachingLocale.h"
#include "Parser.h"
#include "Analyst.h"
#include <QObject>
#include <QTime>

#include <QApplication>
#include "UI/MainWindow.h"


int main(int argc, char *argv[]) {
    QCachingLocale cl;

    // Merely instantiating QCachingLocale activates it. Use Q_UNUSED to prevent
    // compiler warnings.
    Q_UNUSED(cl);

    QApplication app(argc, argv);

    MainWindow * mainWindow = new MainWindow();
    mainWindow->show();

    return app.exec();
}


