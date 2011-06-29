#include <QApplication>

#include "UI/MainWindow.h"
#include "EpisodesParser/QCachingLocale/QCachingLocale.h"

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
