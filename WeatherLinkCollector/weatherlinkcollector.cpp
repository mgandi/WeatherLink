#include "weatherlinkcollector.h"

#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlTableModel>

#include <QDir>

typedef struct {
    QDateTime timeStamp;

    double currentOutsideTemperature;
    double maxOutsideTemperature;
    double minOutsideTemperature;

    quint16 currentOutsideHumidity;
    quint16 maxOutsideHumidity;
    quint16 minOutsideHumidity;

    double currentInsideTemperature;
    double maxInsideTemperature;
    double minInsideTemperature;

    quint16 currentInsideHumidity;
    quint16 maxInsideHumidity;
    quint16 minInsideHumidity;

    double currentHeatIndex;
    double maxHeatIndex;

    double currentWindChill;
    double minWindChill;

    double currentDewPoint;
    double maxDewPoint;
    double minDewPoint;

    double currentPressure;
    double maxPressure;
    double minPressure;

    double currentWindSpeed;
    double maxWindSpeed;

    quint16 currentWindDirection;
    double averageWindSpeed2Minutes;
    double averageWindSpeed10Minutes;
    double windGust;
} WeatherLinkData;

class WeatherLinkCollectorPrivate
{
public:
    WeatherLinkCollectorPrivate(const QString &station, const QUrl &url, const QString &where, const quint32 intervalSeconds, const quint32 depthSeconds) :
        name(station),
        location(url),
        path(where),
        interval(intervalSeconds),
        depth(depthSeconds),
        db(QSqlDatabase::addDatabase("QSQLITE"))
    {
        // Compute path if not provided
        if (path.isEmpty()) {
            path = QString(QDir::home().path());
            path.append(QDir::separator()).append("weatherlink.sqlite");
            path = QDir::toNativeSeparators(path);
        }

        // Set database name
        db.setDatabaseName(path);

        // Open database
        if (!db.open()) {
            qDebug() << qPrintable(db.lastError().text());
        }
    }

    ~WeatherLinkCollectorPrivate()
    {
        db.close();
    }

    int timerId;
    QString name;
    QUrl location;
    QString path;
    quint32 interval, depth;
    QSqlDatabase db;

    QNetworkAccessManager manager;
    QByteArray content;
    WeatherLinkData lastData;
};



WeatherLinkCollector::WeatherLinkCollector(const QString &name, const QUrl &location, const QString &path, quint32 interval, quint32 depth, QObject *parent) :
    QObject(parent),
    d(new WeatherLinkCollectorPrivate(name, location, path, interval, depth))
{
}

WeatherLinkCollector::~WeatherLinkCollector()
{
    delete d;
}


void WeatherLinkCollector::start()
{
    // Start timer
    d->timerId = startTimer(d->interval * 1000);
}


void WeatherLinkCollector::timerEvent(QTimerEvent *e)
{
    Q_UNUSED(e)

    // New capture
    dump();
}

void WeatherLinkCollector::dump()
{
    // Get page content
    QNetworkRequest request(d->location);
    QNetworkReply *reply = d->manager.get(request);
    connect(reply, SIGNAL(finished()),
            this, SLOT(finished()));
}

void WeatherLinkCollector::parse()
{
    WeatherLinkData data;

    // Save current time stamp
    data.timeStamp = QDateTime::currentDateTime();

    qDebug() << "Capture at:" << qPrintable(data.timeStamp.toString(Qt::ISODate));

    // Re format content
    d->content.replace("\r\n", "");
    d->content.replace("\t", "");
    QString start = "<!-- START: SUMMARY WEATHER DISPLAY -->", end = "<!-- END: SUMMARY WEATHER DISPLAY -->";
    int startIndex = d->content.indexOf(start), endIndex = d->content.indexOf(end);
    d->content = d->content.mid(startIndex + start.length(), (endIndex - startIndex - start.length()));

    // Extract data
    QRegExp regex("<td width=\"\\d+\" class=\"\\w+\">([A-Za-z\\s]+)</td>" \
                  "<td width=\"\\d+\" class=\"\\w+\">(([NSEW]+&nbsp;)?(-?\\d+\\.?\\d?)\\s?(C|%|mb|km/h|&deg;)|&nbsp;)</td>" \
                  "<td width=\"\\d+\" class=\"\\w+\">((-?\\d+\\.?\\d?)\\s?(C|%|mb|km/h)|&nbsp;)</td>" \
                  "<td width=\"\\d+\" class=\"\\w+\">(\\d\\d:\\d\\d|&nbsp;)</td>" \
                  "<td width=\"\\d+\" class=\"\\w+\">((-?\\d+\\.?\\d?)\\s?(C|%|mb|km/h)|&nbsp;)</td>" \
                  "<td width=\"\\d+\" class=\"\\w+\">(\\d\\d:\\d\\d|&nbsp;)</td>");
    int pos = 0;

    regex.indexIn(d->content);
    while ((pos = regex.indexIn(d->content, pos)) != -1) {
        pos += regex.matchedLength();

        QStringList captures = regex.capturedTexts();

        if (regex.captureCount() == 13) {
            if (captures[1] == "Outside Temp") {
                data.currentOutsideTemperature = captures[4].toDouble();
                data.maxOutsideTemperature = captures[7].toDouble();
                data.minOutsideTemperature = captures[11].toDouble();
            } else if (captures[1] == "Outside Humidity") {
                data.currentOutsideHumidity = captures[4].toUShort();
                data.maxOutsideHumidity = captures[7].toUShort();
                data.minOutsideHumidity = captures[11].toUShort();
            } else if (captures[1] == "Inside Temp") {
                data.currentInsideTemperature = captures[4].toDouble();
                data.maxInsideTemperature = captures[7].toDouble();
                data.minInsideTemperature = captures[11].toDouble();
            } else if (captures[1] == "Inside Humidity") {
                data.currentInsideHumidity = captures[4].toUShort();
                data.maxInsideHumidity = captures[7].toUShort();
                data.minInsideHumidity = captures[11].toUShort();
            } else if (captures[1] == "Heat Index") {
                data.currentHeatIndex = captures[4].toDouble();
                data.maxHeatIndex = captures[7].toDouble();
            } else if (captures[1] == "Wind Chill") {
                data.currentWindChill = captures[4].toDouble();
                data.minWindChill = captures[11].toDouble();
            } else if (captures[1] == "Dew Point") {
                data.currentDewPoint = captures[4].toDouble();
                data.maxDewPoint = captures[7].toDouble();
                data.minDewPoint = captures[11].toDouble();
            } else if (captures[1] == "Barometer") {
                data.currentPressure = captures[4].toDouble();
                data.maxPressure = captures[7].toDouble();
                data.minPressure = captures[11].toDouble();
            } else if (captures[1] == "Bar Trend") {
            } else if (captures[1] == "Wind Speed") {
                data.currentWindSpeed = captures[4].toDouble();
                data.maxWindSpeed = captures[7].toDouble();
            } else if (captures[1] == "Wind Direction") {
                data.currentWindDirection = captures[4].toUShort();
                if (data.currentWindDirection > 360) {
                    qDebug() << "Error on wind direction:" << qPrintable(captures[0]);
                }
            } else if (captures[1] == "Solar Radiation") {
            } else if (captures[1] == "UV Radiation") {
            } else if (captures[1] == "Average Wind Speed") {
                data.averageWindSpeed2Minutes = captures[4].toDouble();
                data.averageWindSpeed10Minutes = captures[7].toDouble();
            } else if (captures[1] == "Wind Gust Speed") {
                data.windGust = captures[7].toDouble();
            }
        }
    }

    // Save data
    d->lastData = data;

    // Log
    log();
}

void WeatherLinkCollector::log()
{
    QSqlQuery sqlQuery(d->db);

    // Test if the requested table exists
    if (!d->db.tables().contains(d->name, Qt::CaseInsensitive)) {
        qDebug() << "Create table:" << qPrintable(d->name);

        // Create table
        if (!sqlQuery.exec(QString("create table %1"
                                   "(id integer primary key,"

                                   "timeStamp datetime,"
                                   "currentOutsideTemperature double,"
                                   "maxOutsideTemperature double,"
                                   "minOutsideTemperature double,"

                                   "currentOutsideHumidity integer,"
                                   "maxOutsideHumidity integer,"
                                   "minOutsideHumidity integer,"

                                   "currentInsideTemperature double,"
                                   "maxInsideTemperature double,"
                                   "minInsideTemperature double,"

                                   "currentInsideHumidity integer,"
                                   "maxInsideHumidity integer,"
                                   "minInsideHumidity integer,"

                                   "currentHeatIndex double,"
                                   "maxHeatIndex double,"

                                   "currentWindChill double,"
                                   "minWindChill double,"

                                   "currentDewPoint double,"
                                   "maxDewPoint double,"
                                   "minDewPoint double,"

                                   "currentPressure double,"
                                   "maxPressure double,"
                                   "minPressure double,"

                                   "currentWindSpeed double,"
                                   "maxWindSpeed double,"

                                   "currentWindDirection integer,"
                                   "averageWindSpeed2Minutes double,"
                                   "averageWindSpeed10Minutes double,"
                                   "windGust double)").arg(d->name))) {
            qDebug() << qPrintable(sqlQuery.lastError().text());
        }
    }

    // Insert data into table
    if (!sqlQuery.exec(QString("insert into %1 values(NULL, %2, %3, %4, %5, %6, %7, %8, %9, %10, %11, %12, %13, %14, %15, %16, %17, %18, %19, %20, %21, %22, %23, %24, %25, %26, %27, %28, %29, %30)")
                       .arg(d->name)

                       .arg(d->lastData.timeStamp.toMSecsSinceEpoch() / 1000)
                       .arg(d->lastData.currentOutsideTemperature)
                       .arg(d->lastData.maxOutsideTemperature)
                       .arg(d->lastData.minOutsideTemperature)

                       .arg(d->lastData.currentOutsideHumidity)
                       .arg(d->lastData.minOutsideHumidity)
                       .arg(d->lastData.minOutsideHumidity)

                       .arg(d->lastData.currentInsideTemperature)
                       .arg(d->lastData.maxInsideTemperature)
                       .arg(d->lastData.minInsideTemperature)

                       .arg(d->lastData.currentInsideHumidity)
                       .arg(d->lastData.maxInsideHumidity)
                       .arg(d->lastData.minInsideHumidity)

                       .arg(d->lastData.currentHeatIndex)
                       .arg(d->lastData.maxHeatIndex)

                       .arg(d->lastData.currentWindChill)
                       .arg(d->lastData.minWindChill)

                       .arg(d->lastData.currentDewPoint)
                       .arg(d->lastData.maxDewPoint)
                       .arg(d->lastData.minDewPoint)

                       .arg(d->lastData.currentPressure)
                       .arg(d->lastData.maxPressure)
                       .arg(d->lastData.minPressure)

                       .arg(d->lastData.currentWindSpeed)
                       .arg(d->lastData.maxWindSpeed)

                       .arg(d->lastData.currentWindDirection)
                       .arg(d->lastData.averageWindSpeed2Minutes)
                       .arg(d->lastData.averageWindSpeed10Minutes)
                       .arg(d->lastData.windGust))) {
        qDebug() << qPrintable(sqlQuery.lastError().text());
    }

    // Clean all out dated records
    if (!sqlQuery.exec(QString("delete from %1 where timeStamp < %2")
                      .arg(d->name)
                      .arg((d->lastData.timeStamp.toMSecsSinceEpoch() / 1000) - d->depth))) {
        QSqlError error = sqlQuery.lastError();
        if (error.type() != QSqlError::NoError)
            qDebug() << qPrintable(error.text());
    }
}


void WeatherLinkCollector::finished()
{
    // Get associated network reply
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    // Extract page content
    d->content = reply->readAll();

    // Prepare deletion of network reply
    reply->deleteLater();

    // Parse page
    parse();
}

void WeatherLinkCollector::error(QNetworkReply::NetworkError)
{
    // Get associated network reply
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    qDebug() << "Network error:" << qPrintable(reply->errorString());
}
