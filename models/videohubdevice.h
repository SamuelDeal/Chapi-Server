#ifndef VIDEOHUBDEVICE_H
#define VIDEOHUBDEVICE_H

#include <QTimer>

#include "connecteddevice.h"
#include "../utils/nlprotocol.h"

class VideoHubDevice :  public ConnectedDevice
{
    Q_OBJECT

public:
    VideoHubDevice(const Device &dev);
    virtual ~VideoHubDevice();

    bool isConfigurable() const;
    bool isIdentifiable() const;
    Device::DeviceSimpeStatus simpleStatus() const;
    virtual bool isConfigurableNow() const;
    virtual bool isIdentifiableNow() const;
    virtual void blink();

    QMap<quint8, QString> inputLabels() const;
    QMap<quint8, QString> outputLabels() const;

    void setInputName(quint8 index, QString name);
    void setOutputName(quint8 index, QString name);

    virtual void loadSpecific(QSettings &settings);
    virtual void saveSpecific(QSettings &settings);

protected:
    virtual void parseInput();
    virtual void ping();
    virtual quint16 port() const;
    virtual int pingDelay() const;
    virtual quint32 pingLostTolerance() const;
    virtual int reconnectDelay() const;

    void checkEndConfig();

    QMap<quint8, QString> _inputLabels;
    QMap<quint8, QString> _outputLabels;
    QMap<quint8, quint8> _routingTable;

    bool _blinking;
    QTimer _blinkTimer;
    NlProtocol _protocol;

public slots:
    void onBlinked();
    void onBlinkFinished();
    void onCommandFailed(NlCommand cmd);
    void onCommandReceived(NlCommand cmd);
};

#endif // VIDEOHUBDEVICE_H
