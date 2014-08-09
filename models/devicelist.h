#ifndef DEVICELIST_H
#define DEVICELIST_H

#include <qobject.h>
#include <qprocess.h>
#include <qdom.h>
#include <qmap.h>
#include <qthread.h>

#include "device.h"
#include "devicescanner.h"

class DeviceList : public QObject
{
    Q_OBJECT
public:
    explicit DeviceList();
    ~DeviceList();
    void save() const;
    void load();
    void setNmapPath(const QString &path);
    bool containsMac(quint64 mac) const;
    Device* deviceByMac(quint64 mac) const;
    QList<Device *> devices() const;

private:
    Device* generateDevice(const DeviceInfo &);

    QMap<quint64, Device*> _devices;
    QMap<quint64, bool> _previousDevScanList;
    QMap<quint64, bool> _currentDevScanList;
    DeviceScanner _scanner;
    QThread _thread;

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
