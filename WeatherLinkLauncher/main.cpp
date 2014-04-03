#include <QCoreApplication>
#include <QDir>

#include "weatherlinklauncher.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Create path to db
    QString path(QDir::home().path());
    path.append(QDir::separator()).append("weatherlink.sqlite");
    path = QDir::toNativeSeparators(path);

    // Parse arguments
    QStringList args = a.arguments();
    for (int i = 0; i < args.count(); i++) {
        if (((args[i] == "--path") || (args[i] == "-p")) && (args.count() > ++i)) {
            path = args[i];
        }
    }

    // Create and start Weather Link Launcher
    WeatherLinkLauncher launcher(path);
    QObject::connect(&a, SIGNAL(aboutToQuit()),
                     &launcher, SLOT(aboutToQuit()));
    launcher.start();

    return a.exec();
}
