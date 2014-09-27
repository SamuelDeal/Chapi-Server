#ifndef SYSLOGGER_H
#define SYSLOGGER_H

#include <QObject>
#include <QAbstractSocket>
#include <QList>
#include <QDebug>

#include "syslogentry.h"

class QUdpSocket;
class DeviceList;

class SysLogger : public QObject
{
    Q_OBJECT
public:
    explicit SysLogger(DeviceList *devList);
    ~SysLogger();

    DeviceList *devList() const;
    static void logHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    void log(QtMsgType type, const QString &msg);

private:
    QUdpSocket *_udpSocket;
    QString _buffer;
    QTextStream _stream;
    DeviceList *_devList;

    static SysLogger* _currentLogger;

signals:
    void onNewEntry(SyslogEntry);

public slots:
    void onData();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError);
    void onHostFound();
    void onStateChanged(QAbstractSocket::SocketState);
};

#endif // SYSLOGGER_H
