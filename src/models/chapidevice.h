#ifndef CHAPIDEVICE_H
#define CHAPIDEVICE_H

#include <QObject>
#include <qtcpsocket.h>
#include <qtimer.h>

#include "connecteddevice.h"
#include "networkconfig.h"
#include "../utils/nlprotocol.h"

class DeviceList;
class TargetableDevice;

class ChapiDevice: public ConnectedDevice
{
    Q_OBJECT

public:
    static const quint16 NO_SOURCE = 65534;

    enum CnxStatus {
        Unreachable = 1,
        Connecting = 2,
        Connected = 3
    };

    explicit ChapiDevice(const Device &dev, DeviceList *devList);
    virtual ~ChapiDevice();

    void blink();
    void restart();
    bool isConfigurable() const;
    bool isConfigured() const;
    bool isIdentifiable() const;
    virtual bool isLoggable() const;
    virtual bool isUpdatable() const;
    virtual bool isUpdatableNow() const;
    quint16 nbrButtons() const;
    quint16 faderCount() const;
    Device::DeviceSimpeStatus simpleStatus() const;

    void saveConfig(NetworkConfig &);

    NetworkConfig networkConfig() const;
    quint64 targetMac() const;
    QString targetIp() const;
    void setTarget(TargetableDevice *device);
    quint16 outputIndex(quint16 btnIndex) const;
    void setOutputIndex(quint16 btnIndex, quint16 outputIndex);
    quint16 inputIndex(quint16 btnIndex) const;
    void setInputIndex(quint16 btnIndex, quint16 inputIndex);

    bool isConfigurableNow() const;

protected:
    virtual QAbstractSocket *initSocket();

private:
    virtual void parseInput();
    virtual void ping();
    virtual quint16 port() const;
    virtual int pingDelay() const;
    virtual quint32 pingLostTolerance() const;
    virtual int reconnectDelay() const;

    void updateDeviceStatus();

    QTcpSocket _tcpSocket;
    CnxStatus _cnxStatus;
    quint16 _nbrBtns;
    quint16 _faderCount;
    NetworkConfig _netCfg;
    bool _configSet;
    QString _targetIp;
    quint64 _targetMac;
    TargetableDevice *_target;
    DeviceList *_devList;
    QMap<quint16, quint16> _outputsByBtns;
    QMap<quint16, quint16> _inputsByBtns;
    NlProtocol _protocol;

signals:

public slots:
    void onTargetChanged();
    void onCommandFailed(NlCommand cmd);
    void onCommandReceived(NlCommand cmd);
};

#endif // CHAPIDEVICE_H
