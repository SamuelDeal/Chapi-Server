#ifndef TARGETABLEDEVICE_H
#define TARGETABLEDEVICE_H

#include "connecteddevice.h"

#include <QMap>

class TargetableDevice : public ConnectedDevice
{
    Q_OBJECT

public:
    TargetableDevice(const Device &dev);
    virtual ~TargetableDevice();

    virtual bool isTargetable() const;

    virtual QMap<quint16, QString> getInputs() const = 0;
    virtual QMap<quint16, QString> getOutputs() const = 0;

};

#endif // TARGETABLEDEVICE_H
