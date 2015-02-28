#ifndef SYSLOGGER_H
#define SYSLOGGER_H

#include <QObject>
#include <QAbstractSocket>
#include <QList>
#include <QDebug>

#include "syslogentry.h"
#include "syslogmodel.h"

class QUdpSocket;
class DeviceList;

class SysLogger : public QObject
{
    Q_OBJECT
public:
    explicit SysLogger(DeviceList &devList);
    ~SysLogger();

    static void logHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    void log(QtMsgType type, const QString &msg);
    SyslogModel& model();

private:
    DeviceList &_devList;
    SyslogModel _model;
    QUdpSocket *_udpSocket;
    QString _buffer;
    QTextStream _stream;

    static SysLogger* _currentLogger;

public slots:
    void onData();
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError);
    void onHostFound();
    void onStateChanged(QAbstractSocket::SocketState);
};

#endif // SYSLOGGER_H
