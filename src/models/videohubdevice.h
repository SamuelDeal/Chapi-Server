#ifndef VIDEOHUBDEVICE_H
#define VIDEOHUBDEVICE_H

#include <QTimer>

#include "targetabledevice.h"
#include "../utils/nlprotocol.h"

class QAbstractSocket;

class VideoHubDevice :  public TargetableDevice
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

    virtual QMap<quint16, QString> getInputs() const;
    virtual QMap<quint16, QString> getOutputs() const;

    void setInputName(quint16 index, QString name);
    void setOutputName(quint16 index, QString name);

    virtual void loadSpecific(QSettings &settings);
    virtual void saveSpecific(QSettings &settings);

protected:
    virtual QAbstractSocket* initSocket();

    virtual void parseInput();
    virtual void ping();
    virtual quint16 port() const;
    virtual int pingDelay() const;
    virtual quint32 pingLostTolerance() const;
    virtual int reconnectDelay() const;

    void checkEndConfig();

    QMap<quint16, QString> _inputLabels;
    QMap<quint16, QString> _outputLabels;
    QMap<quint16, quint8> _routingTable;

    QTcpSocket _tcpSocket;
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
