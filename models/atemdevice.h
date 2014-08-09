#ifndef ATEMDEVICE_H
#define ATEMDEVICE_H

#include "targetabledevice.h"

class AtemDevice : public TargetableDevice
{
public:
    AtemDevice(const Device &dev);
    virtual ~AtemDevice();
};

#endif // ATEMDEVICE_H
