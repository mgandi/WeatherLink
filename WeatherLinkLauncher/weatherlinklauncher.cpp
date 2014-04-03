#include "weatherlinklauncher.h"

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include <QDebug>
#include <QProcess>
#include <QCoreApplication>
#include <QDir>

class WeatherLinkLauncherPrivate
{
public:
    QString dbPath;
    QList<QProcess *> processes;
};

WeatherLinkLauncher::WeatherLinkLauncher(const QString &dbPath, QObject *parent) :
    QObject(parent),
    d(new WeatherLinkLauncherPrivate)
{
    d->dbPath = dbPath;
}

WeatherLinkLauncher::~WeatherLinkLauncher()
{
    delete d;
}


bool WeatherLinkLauncher::start()
{
    // Open db
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(d->dbPath);

    // Open database
    if (!db.open()) {
        qDebug() << qPrintable(db.lastError().text());
        return false;
    }

    // Dump all stations
    QSqlQuery sqlQuery(db);
    if (!sqlQuery.exec(QString("select * from stations"))) {
        QSqlError error = sqlQuery.lastError();
        if (error.type() != QSqlError::NoError) {
            qDebug() << qPrintable(error.text());
            return false;
        }
    }

    // Launch collectors
    while (sqlQuery.next()) {
        // Get station name
        QString name = sqlQuery.value("name").toString();

        // Get station url
        QString url = sqlQuery.value("url").toString();

        // Create process
        QProcess *process = new QProcess(this);
        connect(process, SIGNAL(error(QProcess::ProcessError)),
                this, SLOT(error(QProcess::ProcessError)));
        connect(process, SIGNAL(finished(int,QProcess::ExitStatus)),
                this, SLOT(finished(int,QProcess::ExitStatus)));
        connect(process, SIGNAL(started()),
                this, SLOT(started()));
        connect(process, SIGNAL(readyReadStandardError()),
                this, SLOT(readyReadStandardError()));
        connect(process, SIGNAL(readyReadStandardOutput()),
                this, SLOT(readyReadStandardOutput()));

        QString command = QString("%1/wl_collector -n %2 -u %3")
                .arg(QCoreApplication::applicationDirPath())
                .arg(name)
                .arg(url);
        process->start(command);
        d->processes += process;
    }

    return true;
}

void WeatherLinkLauncher::aboutToQuit()
{
    fprintf(stderr, "About to quit!");
    QList<QProcess *> tmp;

    foreach (QProcess * process, d->processes) {
        tmp += process;
    }

    foreach (QProcess * process, tmp) {
        process->terminate();
    }

    while (d->processes.count());
}

void WeatherLinkLauncher::error(QProcess::ProcessError)
{
    // Get associated process
    QProcess *process = qobject_cast<QProcess *>(sender());

    // Display error
    qDebug() << "Process error:" << qPrintable(process->program()) << qPrintable(process->arguments().join(" ")) << "-" << qPrintable(process->errorString());

    // Remove process from the list of processes
    d->processes.removeAll(process);
}

void WeatherLinkLauncher::finished(int, QProcess::ExitStatus)
{
    // Get associated process
    QProcess *process = qobject_cast<QProcess *>(sender());

    // Display message
    qDebug() << "Process finished:" << qPrintable(process->program()) << qPrintable(process->arguments().join(" "));

    // Remove process from the list of processes
    d->processes.removeAll(process);
}

void WeatherLinkLauncher::started()
{
    // Get associated process
    QProcess *process = qobject_cast<QProcess *>(sender());

    // Display message
    qDebug() << "Process started:" << qPrintable(process->program()) << qPrintable(process->arguments().join(" "));
}

void WeatherLinkLauncher::readyReadStandardError()
{
    // Get associated process
    QProcess *process = qobject_cast<QProcess *>(sender());

    // Print to stderr
    fprintf(stderr, qPrintable(QString(process->readAllStandardError())));
}

void WeatherLinkLauncher::readyReadStandardOutput()
{
    // Get associated process
    QProcess *process = qobject_cast<QProcess *>(sender());

    // Print to stderr
    fprintf(stdout, qPrintable(QString(process->readAllStandardOutput())));
}
