#include "weatherlinkcollector.h"

#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString name;
    QString url;
    bool displayHelp = false;

    // Parse arguments
    QStringList args = a.arguments();
    for (int i = 0; i < args.count(); i++) {
        if (((args[i] == "--name") || (args[i] == "-n")) && (args.count() > ++i)) {
            name = args[i];
        } else if (((args[i] == "--url") || (args[i] == "-u")) && (args.count() > ++i)) {
            url = args[i];
        } else if ((args[i] == "--help") || (args[i] == "-h")) {
            displayHelp = true;
        }
    }

    // Test that we have all arguments
    if (name.isEmpty() || url.isEmpty() || displayHelp) {
        QString help =
                "WeatherLink Data Collector\n"
                "Options:\n"
                " -n --name: Name of meteo station\n"
                " -u --url:  Url of meteo station\n"
                " -h --help: Display current message\n";

        qDebug() << qPrintable(help);
        return 0;
    }

    // Start collector
    WeatherLinkCollector collector(name, QUrl(url));
    collector.start();

    return a.exec();
}
