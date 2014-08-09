#ifndef TARGETABLEDEVICE_H
#define TARGETABLEDEVICE_H

#include "connecteddevice.h"

class TargetableDevice : public ConnectedDevice
{
public:
    TargetableDevice(const Device &dev);
    virtual ~TargetableDevice();
};

#endif // TARGETABLEDEVICE_H
