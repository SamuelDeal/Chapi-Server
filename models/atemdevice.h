#ifndef ATEMDEVICE_H
#define ATEMDEVICE_H

#include "targetabledevice.h"

#include <QUdpSocket>

class AtemDevice : public TargetableDevice
{
    Q_OBJECT

public:
    enum Outputs {
        Program = 1,
        Preview = 2,
        ME2 = 3,
        ME2Preview = 4,
        AuxFirst = 5
    };

    AtemDevice(const Device &dev);
    virtual ~AtemDevice();

    virtual QMap<quint16, QString> getInputs() const;
    virtual QMap<quint16, QString> getOutputs() const;
    virtual bool isConfigurableNow() const;
    virtual bool isConfigurable() const;

    void setInputName(quint16 index, QString name);
    virtual void loadSpecific(QSettings &settings);
    virtual void saveSpecific(QSettings &settings);

protected:
    virtual QAbstractSocket *initSocket();
    virtual void parseInput();
    virtual void ping();
    virtual quint16 port() const;
    virtual int pingDelay() const;
    virtual quint32 pingLostTolerance() const;
    virtual int reconnectDelay() const;
    virtual void onCnxEstablished();

    void sendAck(quint16 remotePacketID);
    void parsePacket(quint16 packetLength);
    bool ver42();
    void changeProgramInput(quint16 inputNumber);
    void sendPacketBufferCmdData(const char cmd[4], const QByteArray &data);
    void changeInputName(quint16 input, const QString &name);
    void setTransitionValue(quint16 value);

    bool _isMe2;
    QByteArray _buffer;
    quint16 _sessionID;
    quint16 _nbrAux;
    bool _helloing;
    bool _hasInitialized;
    QUdpSocket _udpSocket;
    quint16 _localPacketIdCounter;
    quint16 _token;

    QMap<quint16, QString> _inputLabels;
    QMap<quint16, QString> _outputLabels;

public slots:
    void onTimer();
};

#endif // ATEMDEVICE_H
