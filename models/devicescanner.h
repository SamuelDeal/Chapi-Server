#ifndef DEVICESCANNER_H
#define DEVICESCANNER_H

#include <qobject.h>
#include <qstringlist.h>
#include <qmap.h>
#include <qhostaddress.h>
#include <qprocess.h>
#include <QDomNode>
#include <qudpsocket.h>
#include <qtimer.h>
#include <QElapsedTimer>

#include "device.h"

struct DeviceInfo {
    quint64 mac;
    QString ip;
    QString name;
    Device::DeviceStatus status;
    Device::DeviceType type;
    QString version;
    bool currentComputer;
};

Q_DECLARE_METATYPE(DeviceInfo)

class DeviceScanner : public QObject
{
    Q_OBJECT
public:
    explicit DeviceScanner();
    ~DeviceScanner();

private:
    bool checkNmap(const QString &path);
    Device::DeviceType guessType(bool cfgPort, bool vhPort, bool atemPort, bool serverPort, const QString &ip);
    void parseScanResult(const QDomNode &devNode);
    void cleanProc(QProcess *proc);
    void initFromInterface(const QNetworkInterface &);

    QString _nmapPath;
    QStringList _defaultPaths;
    QMap<quint32, QPair<QHostAddress, int> > _networks;
    QMap<quint32, QProcess*> _scanners;
    QUdpSocket _udpSocket;
    QTimer _timer;
    QElapsedTimer _lastHello;
    quint64 _currentMac;
    QString _currentIp;
    QString _currentMask;

signals:
    void needNmap();
    void needRoot();
    void onScanResult();
    void allScanFinished();
    void deviceDetected(DeviceInfo);

public slots:
    void start();
    void sayHello();
    void nmapPathDefined(QString path);
    void scan();
    void error(QProcess::ProcessError err);
    void finished(int exitCode, QProcess::ExitStatus exitStatus);
    void helloReceived();
};

#endif // DEVICESCANNER_H
