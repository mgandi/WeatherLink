#ifndef WEATHERLINKLAUNCHER_H
#define WEATHERLINKLAUNCHER_H

#include <QObject>
#include <QProcess>

class WeatherLinkLauncherPrivate;
class WeatherLinkLauncher : public QObject
{
    Q_OBJECT
public:
    explicit WeatherLinkLauncher(const QString &dbPath, QObject *parent = 0);
    ~WeatherLinkLauncher();

    bool start();

public slots:
    void aboutToQuit();

protected slots:
    void error(QProcess::ProcessError error);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void started();
    void readyReadStandardError();
    void readyReadStandardOutput();

private:
    WeatherLinkLauncherPrivate *d;
};

#endif // WEATHERLINKLAUNCHER_H
