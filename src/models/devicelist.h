#ifndef DEVICELIST_H
#define DEVICELIST_H

#include <qobject.h>
#include <qprocess.h>
#include <qdom.h>
#include <qmap.h>
#include <qthread.h>

#include "device.h"
#include "devicescanner.h"

class TargetableDevice;

class DeviceList : public QObject
{
    Q_OBJECT
public:
    explicit DeviceList();
    ~DeviceList();
    void startScans();
    void save() const;
    void load();
    void setNmapPath(const QString &path);
    bool containsMac(quint64 mac) const;
    Device* deviceByMac(quint64 mac) const;
    Device* currentDevice() const;
    TargetableDevice* targetableDeviceByMac(quint64 mac) const;
    QList<Device *> devices() const;
    QList<TargetableDevice *> getTargetableDevices() const;

private:
    bool _started;
    Device* generateDevice(const DeviceInfo &, bool currentComputer = false);
    Device *_currentDev;
    QMap<quint64, Device*> _devices;
    QMap<quint64, bool> _previousDevScanList;
    QMap<quint64, bool> _currentDevScanList;
    DeviceScanner _scanner;

signals:
    void deviceListChanged();
    void needNmap();
    void needRoot();

private Q_SLOTS:
    void scanNeedNmap();
    void onDeviceDetected(DeviceInfo);
    void onAllScansFinished();
};

#endif // DEVICELIST_H
