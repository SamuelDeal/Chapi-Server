#include "device.h"

#include <QSettings>

#include "../utils/netutils.h"
#include "atemdevice.h"
#include "chapidevice.h"
#include "videohubdevice.h"

unsigned int Device::_indexCount = 0;

Device::Device(quint64 mac, QString name, QString ip, DeviceStatus status, DeviceType type) :
    QObject(NULL)
{
    _mac = mac;
    _name = name;
    _ip = ip;
    _status = status;
    _type = type;
    _monitored = false;
    _index = _indexCount++;
    _version = "";

/*    if(type == Device::ChapiDev){
        _chapi = new ChapiDevice();
        connect(_chapi, SIGNAL(statusChanged(Device::DeviceStatus)), this, SLOT(onCnxStatusChanged(Device::DeviceStatus)));
        connect(_chapi, SIGNAL(resetIp()), this, SLOT(onResetIp()));
        if(ip != ""){
            _chapi->connectToDevice(ip);
        }
    }
    else if(type == Device::VideoHub){
        _videoHub = new VideoHubDevice();
    }
    else if(type == Device::Atem) {
        _atem = new AtemDevice();
    }*/
}

Device::Device(const Device &dev) : QObject(NULL) {
    _mac = dev._mac;
    _name = dev._name;
    _ip = dev._ip;
    _status = dev._status;
    _type = dev._type;
    _monitored = dev._monitored;
}

Device::~Device() {
}

void Device::init() {
}

quint64 Device::mac() const {
    return _mac;
}

QString Device::name() const {
    return _name;
}

void Device::setName(const QString &name) {
    _name = name;
    emit changed();
}

QString Device::ip() const {
    return _ip;
}

void Device::setIp(const QString &ip) {
    if(_ip == ip){
        return;
    }
    _ip = ip;
    _status = ip.isEmpty() ? Device::Unreachable : Device::Located;
    emit changed();
}

void Device::onResetIp() {
    bool hasChanged = false;
    if(!_ip.isEmpty()){
        _ip = "";
        hasChanged = true;
    }
    if(_status != Device::Unreachable) {
        _status = Device::Unreachable;
        hasChanged = true;
    }
    if(hasChanged){
        emit changed();
    }
}
QString Device::version() const {
    return _version;
}

void Device::setVersion(const QString &version) {
    if(version == _version){
        return;
    }
    _version = version;
    emit changed();
}


unsigned int Device::index() const {
    return _index;
}

Device::DeviceStatus Device::status() const {
    return _status;
}

void Device::setStatus(DeviceStatus status){
    _status = status;
    emit changed();
}

bool Device::isConfigurable() const {
    return false;
}

bool Device::isIdentifiable() const {
    return false;
}

bool Device::isTargetable() const {
    return false;
}

bool Device::isLoggable() const {
    return true;
}

bool Device::isConfigurableNow() const {
    return isConfigurable() && ((_status == Device::Connected) || (_status == Device::Unconfigured) ||
        (_status == Device::HubUnreachable) || (_status == Device::ConnectingToHub) || (_status == Device::Ready));
}

bool Device::isIdentifiableNow() const {
    return isIdentifiable() && ((_status == Device::Connected) || (_status == Device::Unconfigured) ||
        (_status == Device::HubUnreachable) || (_status == Device::ConnectingToHub) || (_status == Device::Ready));
}

bool Device::isUpdatableNow() const {
    return isUpdatable() && ((_status == Device::Connected) || (_status == Device::Unconfigured) ||
        (_status == Device::HubUnreachable) || (_status == Device::ConnectingToHub) || (_status == Device::Ready));
}

bool Device::isCurrentComputer() const {
    return false;
}

bool Device::isMonitorable() const {
    return true;
}

bool Device::isUpdatable() const {
    return false;
}

bool Device::isMonitored() const{
    return isMonitorable() && _monitored;
}

Device::DeviceType Device::type() const {
    return _type;
}

void Device::onCnxStatusChanged(Device::DeviceStatus newStatus) {
    if(newStatus == _status){
        return;
    }
    _status = newStatus;
    emit changed();
}

void Device::setMonitored(bool monitored) {
    _monitored = monitored;
    emit changed();
}

void Device::blink() {
}

Device::DeviceSimpeStatus Device::simpleStatus() const {
    if(_status == Unreachable) {
        return Device::Red;
    }
    else if(_status == Device::Located) {
        if((_type == Device::ChapiDev) || (_type == Device::VideoHub)) {
            return Device::Red;
        }
        else {
            return Device::Green;
        }
    }
    else if(_status == Device::Connecting) {
        return Device::Yellow;
    }
    else if((_status == Device::Connected) && (_type == Device::VideoHub)){
        return Device::Green;
    }
    else if(_status == Device::Ready){
        return Device::Green;
    }
    else {
        return Device::Yellow;
    }
}

void Device::loadSpecific(QSettings &settings) {
    Q_UNUSED(settings);
}

void Device::saveSpecific(QSettings &settings) {
    Q_UNUSED(settings);
}
