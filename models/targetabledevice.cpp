#include "targetabledevice.h"

#include <iostream>

TargetableDevice::TargetableDevice(const Device &dev) :
    ConnectedDevice(dev) {
}


TargetableDevice::~TargetableDevice(){
    std::cerr << "TargetableDevice => deleting " << std::hex << (quint64)this << std::endl;
}

bool TargetableDevice::isTargetable() const {
    return true;
}
