#ifndef SERVERDEVICE_H
#define SERVERDEVICE_H

#include "device.h"

class ServerDevice : public Device
{
    Q_OBJECT

public:
    ServerDevice(const Device &dev, bool isCurrentComputer);

    bool isCurrentComputer() const;
    DeviceSimpeStatus simpleStatus() const;

    virtual void setIp(const QString &ip);
    bool isMonitorable() const;
    bool isIdentifiable() const;

private:
    bool _currentComputer;
};

#endif // SERVERDEVICE_H
