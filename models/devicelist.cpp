#include "devicelist.h"

#include <qdebug.h>
#include <qsettings.h>
#include <qhostaddress.h>
#include <qtimer.h>
#include <qsettings.h>
#include <QStandardPaths>

#include "device.h"
#include "chapidevice.h"
#include "serverdevice.h"
#include "videohubdevice.h"
#include "../utils/netutils.h"

DeviceList::DeviceList() :
    QObject(NULL)
{
    qRegisterMetaType<DeviceInfo>("DeviceInfo");

    _thread.start();
    _scanner.moveToThread(&_thread);
    connect(&_scanner, SIGNAL(needNmap()), this, SIGNAL(needNmap()));
    connect(&_scanner, SIGNAL(needRoot()), this, SIGNAL(needRoot()));
    connect(&_scanner, SIGNAL(deviceDetected(DeviceInfo)), this, SLOT(onDeviceDetected(DeviceInfo)));
    connect(&_scanner, SIGNAL(allScanFinished()), this, SLOT(onAllScansFinished()));
    QTimer::singleShot(100, &_scanner, SLOT(start()));
}

DeviceList::~DeviceList() {
    foreach(Device *dev, _devices){
        delete dev;
    }
    _thread.quit();
    _thread.wait();
}

void DeviceList::scanNeedNmap() {
    emit needNmap();
}

void DeviceList::onDeviceDetected(DeviceInfo devInfo) {
    if(_currentDevScanList.contains(devInfo.mac)){
        _currentDevScanList[devInfo.mac] = true;
    }
    else{
        _currentDevScanList.insert(devInfo.mac, true);
    }

    bool changed = false;
    Device *dev = _devices.value(devInfo.mac, NULL);
    if(dev == NULL){
        dev = generateDevice(devInfo);
        _devices.insert(devInfo.mac, dev);
        changed = true;
    }
    else{
        if(!devInfo.name.isEmpty() && dev->name().isEmpty()){
            dev->setName(devInfo.name);
            changed = true;
        }
        if(devInfo.ip != dev->ip()){
            dev->setIp(devInfo.ip);
            changed = true;
        }
        if((devInfo.version != dev->version()) && !devInfo.version.isEmpty()){
            dev->setVersion(devInfo.version);
            changed = true;
        }
        if((dev->status() == Device::Unreachable) && (devInfo.status != dev->status())){
            dev->setStatus(devInfo.status);
            changed = true;
        }
    }
    if(changed){
        emit deviceListChanged();
    }
}

void DeviceList::save() const{
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);
    foreach(QString key, settings.allKeys()) {
        if(key.startsWith("Device_")) {
            settings.remove(key);
        }
    };
    foreach(Device *dev, _devices) {
        if(!dev->isCurrentComputer() && (dev->type() != Device::UnknownDevice)) {
            settings.beginGroup("Device_"+QString::number(dev->mac()));
            settings.setValue("name", dev->name());
            settings.setValue("mac", NetUtils::macToStr(dev->mac()));
            settings.setValue("type", (quint8)dev->type());
            dev->saveSpecific(settings);
            settings.endGroup();
        }
    }
}

Device* DeviceList::generateDevice(const DeviceInfo &devInfo) {
    switch(devInfo.type){
        case Device::ChapiDev:          return new ChapiDevice(Device(devInfo.mac, devInfo.name, devInfo.ip, devInfo.status, devInfo.type), this);
        case Device::ChapiServer:       return new ServerDevice(Device(devInfo.mac, devInfo.name, devInfo.ip, devInfo.status, devInfo.type), devInfo.currentComputer);
        case Device::VideoHub:          return new VideoHubDevice(Device(devInfo.mac, devInfo.name, devInfo.ip, devInfo.status, devInfo.type));

        case Device::Atem:
        case Device::UnknownDevice:
        default:
            return new Device(devInfo.mac, devInfo.name, devInfo.ip, devInfo.status, devInfo.type);
    }
}

void DeviceList::load() {
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);
    QStringList deviceGroups = settings.childGroups();

    bool changed = false;
    foreach(QString deviceGroup, deviceGroups) {
        settings.beginGroup(deviceGroup);
        QString name = settings.value("name", "").toString();
        QString macStr = settings.value("mac", "").toString();
        quint8 type = settings.value("type", Device::UnknownDevice).toUInt();
        quint64 mac = NetUtils::strToMac(macStr);

        if(mac != 0){
            Device *dev = _devices.value(mac, NULL);
            if(dev == NULL){
                DeviceInfo devInfo;
                devInfo.mac = mac;
                devInfo.name = name;
                devInfo.status = Device::Unreachable;
                devInfo.type = (Device::DeviceType)type;

                dev = generateDevice(devInfo);
                dev->loadSpecific(settings);
                _devices.insert(mac, dev);
                changed = true;
            }
            else{
                if(!name.isEmpty() && dev->name().isEmpty()){
                    dev->setName(name);
                    changed = true;
                }
            }
        }
        settings.endGroup();
    }

    if(changed){
        emit deviceListChanged();
    }
}

bool DeviceList::containsMac(quint64 mac) const {
    return _devices.contains(mac);
}

Device* DeviceList::deviceByMac(quint64 mac) const {
    if(!containsMac(mac)){
        return NULL;
    }
    return _devices[mac];
}

QList<Device*> DeviceList::devices() const {
    QList<Device*> result;
    foreach(Device* dev, _devices){
        result.push_back(dev);
    }
    qSort(result.begin(), result.end(), [](Device *a, Device *b){
        return a->index() > b->index();
    });
    return result;
}

void DeviceList::setNmapPath(const QString &path) {
    QString pathCopy = path;
    QMetaObject::invokeMethod(&_scanner, "nmapPathDefined", Qt::QueuedConnection, Q_ARG(QString, pathCopy));
}

void DeviceList::onAllScansFinished() {
    foreach(quint64 mac, _devices.keys()) {
        if(!_currentDevScanList.contains(mac) && !_previousDevScanList.contains(mac)) {
            _devices[mac]->setIp("");
        }
    }
    _previousDevScanList = _currentDevScanList;
    _currentDevScanList.clear();
}
