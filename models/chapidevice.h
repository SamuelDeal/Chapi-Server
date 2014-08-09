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
    static const quint8 NO_SOURCE = 255;

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
    quint8 nbrButtons() const;
    Device::DeviceSimpeStatus simpleStatus() const;

    void saveConfig(NetworkConfig &);

    NetworkConfig networkConfig() const;
    quint64 targetMac() const;
    QString targetIp() const;
    void setTarget(TargetableDevice *device);
    quint8 outputIndex() const;
    void setOutputIndex(quint8 outputIndex);
    quint8 inputIndex(quint8 btnIndex) const;
    void setInputIndex(quint8 btnIndex, quint8 inputIndex);

private:
    virtual void parseInput();
    virtual void ping();
    virtual quint16 port() const;
    virtual int pingDelay() const;
    virtual quint32 pingLostTolerance() const;
    virtual int reconnectDelay() const;

    void updateDeviceStatus();

    CnxStatus _cnxStatus;
    quint8 _nbrBtns;
    NetworkConfig _netCfg;
    bool _configSet;
    QString _targetIp;
    quint64 _targetMac;
    TargetableDevice *_target;
    DeviceList *_devList;
    quint8 _outputIndex;
    QMap<quint8, quint8> _inputIndexes;
    NlProtocol _protocol;

signals:

public slots:
    void onTargetChanged();
    void onCommandFailed(NlCommand cmd);
    void onCommandReceived(NlCommand cmd);
};

#endif // CHAPIDEVICE_H
