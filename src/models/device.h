#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QTcpSocket>
#include <stdint.h>

class AtemDevice;
class VideoHubDevice;
class ChapiDevice;
class QSettings;

class Device : public QObject
{
    Q_OBJECT

public:
    enum DeviceStatus  : quint8 {
        Unreachable = 1,
        Unconfigured,
        CurrentComputer,
        Located,
        Connecting,
        Connected,
        ReadingConfig,
        ApplyingConfig,
        HubUnreachable,
        ConnectingToHub,
        Ready,
        Updating
    };

    enum DeviceType : quint8 {
        ChapiServer = 1,
        ChapiDev,
        VideoHub,
        Atem,
        Router,
        UnknownDevice
    };

    enum DeviceSimpeStatus : quint8 {
        Red = 1,
        Yellow,
        Green
    };


    explicit Device(quint64 mac, QString name, QString ip, DeviceStatus status, DeviceType type);
    Device(const Device &);
    virtual ~Device();

    virtual void init();

    QString name() const;
    void setName(const QString &name);
    QString ip() const;
    virtual void setIp(const QString &ip);
    DeviceStatus status() const;
    void setStatus(DeviceStatus status);
    quint64 mac() const;
    DeviceType type() const;
    virtual DeviceSimpeStatus simpleStatus() const;
    QString version() const;
    void setVersion(const QString &version);
    unsigned int index() const;
    virtual bool isTargetable() const;
    virtual bool isLoggable() const;
    virtual bool isCurrentComputer() const;
    virtual bool isMonitorable() const;
    virtual bool isMonitored() const;
    virtual bool isConfigurable() const;
    virtual bool isIdentifiable() const;
    virtual bool isUpdatable() const;
    virtual bool isConfigurableNow() const;
    virtual bool isIdentifiableNow() const;
    virtual bool isUpdatableNow() const;
    virtual void loadSpecific(QSettings &settings);
    virtual void saveSpecific(QSettings &settings);

    void setMonitored(bool);
    virtual void blink();

signals:
    void changed();

public slots:
    void onCnxStatusChanged(Device::DeviceStatus);
    void onResetIp();

protected:
    static unsigned int _indexCount;

    unsigned int _index;
    quint64 _mac;
    QString _name;
    QString _ip;
    DeviceStatus _status;
    DeviceType _type;
    QString _version;
    bool _monitored;
};

#endif // DEVICE_H
