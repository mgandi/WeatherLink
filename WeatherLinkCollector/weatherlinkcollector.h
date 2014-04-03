#ifndef WEATHERLINKCOLLECTOR_H
#define WEATHERLINKCOLLECTOR_H

#include <QObject>
#include <QUrl>
#include <QtNetwork/QNetworkReply>

class WeatherLinkCollectorPrivate;

class WeatherLinkCollector : public QObject
{
    Q_OBJECT
public:
    WeatherLinkCollector(const QString &name, const QUrl &location, quint32 interval = 30, quint32 depth = 7200, QObject *parent = 0);
    ~WeatherLinkCollector();

    void start();

protected slots:
    void timerEvent(QTimerEvent *e);
    void dump();
    void parse();
    void log();

private slots:
    void finished();
    void error(QNetworkReply::NetworkError code);

private:
    WeatherLinkCollectorPrivate *d;

};

#endif // WEATHERLINKCOLLECTOR_H
