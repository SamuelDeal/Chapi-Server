#ifndef SYSLOGENTRY_H
#define SYSLOGENTRY_H

#include <QStringList>
#include <QDateTime>

class Device;

struct SyslogEntry
{
public:
    SyslogEntry(const QStringList &entryData, Device *dev);
    SyslogEntry(QtMsgType type, const QString &msg, Device* dev);

    quint32 facility() const;
    quint8 severity() const;
    QDateTime date() const;
    Device *host() const;
    QString app() const;
    QString msg() const;

    QString severityName() const;
    QString facilityName() const;

private:
    quint32 _facility;
    quint8 _severity;
    QDateTime _date;
    Device *_host;
    QString _app;
    QString _msg;
};

#endif // SYSLOGENTRY_H
