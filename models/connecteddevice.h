#ifndef CONNECTEDDEVICE_H
#define CONNECTEDDEVICE_H

#include <QTimer>

#include "device.h"

class QSettings;


class ConnectedDevice : public Device
{
    Q_OBJECT

public:
    ConnectedDevice(const Device &dev);
    virtual ~ConnectedDevice();
    virtual void setIp(const QString &ip);
    QString lastKnownIp() const;

    void pausePing();
    void resumePing();

    virtual void loadSpecific(QSettings &settings);
    virtual void saveSpecific(QSettings &settings);

protected:
    virtual void parseInput() = 0;
    virtual void ping() = 0;
    virtual quint16 port() const = 0;
    virtual int pingDelay() const = 0;
    virtual quint32 pingLostTolerance() const = 0;
    virtual int reconnectDelay() const = 0;

    void parseLine(const QString &line);
    void makeConnection();
    void closeCnx(bool reconnect);

    QTcpSocket _socket;
    QString _lastIp;
    QTimer _reconnectTimer;
    QTimer _pingTimer;
    quint32 _pingSent;

signals:
    void resetIp();

public slots:
    void connectToDevice();
    void onData();
    void onError(QAbstractSocket::SocketError error);
    void onReconnectDelayExpired();
    void onConnected();
    void onPing();
};

#endif // CONNECTEDDEVICE_H
