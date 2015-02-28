#include "targetabledevice.h"

TargetableDevice::TargetableDevice(const Device &dev) :
    ConnectedDevice(dev) {
}

TargetableDevice::~TargetableDevice(){
}

bool TargetableDevice::isTargetable() const {
    return true;
}
