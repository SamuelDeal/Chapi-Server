#include "serverdevice.h"

#include "../const.h"

ServerDevice::ServerDevice(const Device &dev, bool isCurrentComputer) :
    Device(dev) {
    _currentComputer = isCurrentComputer;
    if(isCurrentComputer){
        _version = CURRENT_VERSION;
    }
}

bool ServerDevice::isCurrentComputer() const {
    return _currentComputer;
}

void ServerDevice::setIp(const QString &ip){
    Q_UNUSED(ip);
    return; //ignore ip reset
}

bool ServerDevice::isIdentifiable() const {
    return true;
}

bool ServerDevice::isMonitorable() const {
    return !_currentComputer;
}

bool ServerDevice::isLoggable() const {
    return true;
}

Device::DeviceSimpeStatus ServerDevice::simpleStatus() const {
    if(_currentComputer) {
        return Device::Green;
    }
    if(_status == Unreachable) {
        return Device::Red;
    }
    return Device::Green;
}
