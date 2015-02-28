#include "devicescanner.h"

#include <qfile.h>
#include <qsettings.h>
#include <qnetworkinterface.h>
#include <qprocess.h>
#include <QStandardPaths>
#ifndef Q_OS_WIN32
    #include <unistd.h>
#endif


#include "../const.h"
#include "../utils/netutils.h"

DeviceScanner::DeviceScanner() :
    QObject(NULL), _udpSocket(this), _timer(this)
{
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);
#ifdef Q_OS_WIN32
    _defaultPaths.append("C:\\Program Files\\Nmap");
    _defaultPaths.append("C:\\Program Files (x86)\\Nmap");
#else
    _defaultPaths.append("/bin");
    _defaultPaths.append("/usr/bin");
    _defaultPaths.append("/usr/local/bin");
#endif
    _lastHello.invalidate();

    _currentMac = 0;
    _currentMask = "255.255.255.0";
    _currentIp = "127.0.0.1";
    initFromInterface(NetUtils::getPreferedInterface());
    connect(&_timer, SIGNAL(timeout()), this, SLOT(sayHello()));
}

void DeviceScanner::initFromInterface(const QNetworkInterface &netInterface) {
    _currentIp = NetUtils::getIp(netInterface).toString();
    _currentMac = NetUtils::strToMac(netInterface.hardwareAddress());
    _currentMask = NetUtils::getMask(netInterface).toString();
}

DeviceScanner::~DeviceScanner() {
    foreach(QProcess *proc, _scanners){
        disconnect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
        disconnect(proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
        proc->close();
        proc->kill();
        delete(proc);
    }
}

void DeviceScanner::start() {
    _udpSocket.bind(CHAPI_BROADCAST_PORT, QUdpSocket::ShareAddress);
    connect(&_udpSocket, SIGNAL(readyRead()), this, SLOT(helloReceived()));

    sayHello();
    _timer.start(20000);

    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);
    if(checkNmap(settings.value("General/nmap_path", "").toString())){
        scan();
    }
    else {
        emit needNmap();
    }
}


void DeviceScanner::sayHello() {
    if(_lastHello.isValid() && _lastHello.elapsed() < 1000){
        return; // skip hello if we just said it (to avoid hello communication loop)
    }
    QString msg = ("HELLO " DEV_CHAPI_SERVER " " CURRENT_VERSION " ") + NetUtils::macToStr(_currentMac) + " " + _currentIp + "\n";
    QByteArray datagram = msg.toLatin1();
    QHostAddress broadcastAddr = NetUtils::getBroadcast(QHostAddress(_currentIp), QHostAddress(_currentMask));
    _udpSocket.writeDatagram(datagram.data(), datagram.size(), broadcastAddr, CHAPI_BROADCAST_PORT);
    _lastHello.start();
}

bool DeviceScanner::checkNmap(const QString &path) {
#ifndef Q_OS_WIN32
    if(getuid()) {
        emit needRoot();
        return true;
    }
#endif

#ifdef Q_OS_WIN32
    QString exe = "/nmap.exe";
#else
    QString exe = "/nmap";
#endif
    foreach(QString defaultPath, _defaultPaths){
        QFile nmap(defaultPath + exe);
        if(nmap.exists()) {
            _nmapPath = defaultPath;
            return true;
        }
    }

    if(path == ""){
        return false;
    }
    else{
        QFile nmap(path + exe);
        return nmap.exists();
    }
}

void DeviceScanner::nmapPathDefined(QString path) {
    if(!checkNmap(path)){
        emit needNmap();
    }
    else {
        _nmapPath = path;
        QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);
        settings.setValue("General/nmap_path", path);
        scan();
    }
}

void DeviceScanner::scan() {
    QMap<quint32, QPair<QHostAddress, int> > detectedNetworks;
    foreach(QNetworkInterface inet, QNetworkInterface::allInterfaces()) {
        if(((inet.flags() & QNetworkInterface::IsLoopBack) == 0) && ((inet.flags() & QNetworkInterface::IsUp) == QNetworkInterface::IsUp)) {
            QHostAddress network = NetUtils::getNetwork(inet);
            if(!network.isNull() && (network.protocol() == QAbstractSocket::IPv4Protocol)){
                qDebug() << NetUtils::getIp(inet) << ":" << inet.hardwareAddress();
                int netMask = NetUtils::getMaskPrefix(inet);
                if(!_networks.contains(network.toIPv4Address())){
                    _networks.insert(network.toIPv4Address(), QPair<QHostAddress, int>(network, netMask));
                }
                if(detectedNetworks.contains(network.toIPv4Address())) {
                    detectedNetworks[network.toIPv4Address()] =  QPair<QHostAddress, int>(network,netMask);
                }
                else {
                    detectedNetworks.insert(network.toIPv4Address(), QPair<QHostAddress, int>(network,netMask));
                }
            }
        }
    }

    foreach(quint32 netIp, _networks.keys()){
        if(!detectedNetworks.contains(netIp)){
            //emit interfaceRemoved(_networks[netIp].first.toIPv4Address());
            _networks.remove(netIp);
            if(_scanners.contains(netIp)){
                disconnect(_scanners[netIp], SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
                disconnect(_scanners[netIp], SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
                _scanners[netIp]->close();
                delete(_scanners[netIp]);
                _scanners.remove(netIp);
            }
        }
        else if(!_scanners.contains(netIp)){
            QProcess *scanner = new QProcess();
            QStringList args;
            args << "-sU" << "-sS" << "-oX" << "-" << (_networks[netIp].first.toString()+"/"+QString::number(_networks[netIp].second)) <<
                    "-p" STRINGIFY(CHAPI_TCP_PORT) "," STRINGIFY(CHAPISERVER_UDP_PORT) "," STRINGIFY(ATEM_PORT) "," STRINGIFY(VIDEOHUB_PORT);
#ifdef Q_OS_WIN32
            QString exe = "/nmap.exe";
#else
            QString exe = "/nmap";
#endif

            connect(scanner, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
            connect(scanner, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));

            scanner->setWorkingDirectory(_nmapPath);
            _scanners.insert(netIp, scanner);
            scanner->start("\"" + _nmapPath + exe + "\"", args, QIODevice::ReadOnly);
        }
    }
}


void DeviceScanner::error(QProcess::ProcessError err){

    //Q_UNUSED(err);
    QProcess *proc = (QProcess *)sender();
    qDebug() << "process error :" << err << " " << proc->errorString();

    for (auto it = _scanners.begin(); it != _scanners.end();){
        if (it.value() == proc) {
            it = _scanners.erase(it);
        }
        else {
            ++it;
        }
    }
    disconnect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
    disconnect(proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
    proc->close();
    proc->deleteLater();
}

void DeviceScanner::finished(int exitCode, QProcess::ExitStatus exitStatus){
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    QProcess *proc = (QProcess *)sender();

    QDomDocument doc;
    QString result = proc->readAllStandardOutput();
    doc.setContent(result);
    QDomElement root = doc.documentElement();
    QDomNodeList devList = root.elementsByTagName("host");
    int devListCount = devList.count();
    for(int i = 0; i < devListCount; i++) {
        parseScanResult(devList.at(i));
    }
    cleanProc(proc);
}

void DeviceScanner::cleanProc(QProcess *proc) {
    for (auto it = _scanners.begin(); it != _scanners.end();){
        if (it.value() == proc) {
            it = _scanners.erase(it);
        }
        else {
            ++it;
        }
    }
    disconnect(proc, SIGNAL(error(QProcess::ProcessError)), this, SLOT(error(QProcess::ProcessError)));
    disconnect(proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(finished(int,QProcess::ExitStatus)));
    proc->close();
    delete(proc);

    if(_scanners.empty()){
        emit allScanFinished();
        QTimer::singleShot(1000, this, SLOT(scan()));
    }
}

void DeviceScanner::parseScanResult(const QDomNode &devNode) {
    QDomNodeList devInfo = devNode.toElement().elementsByTagName("address");

    QString name = "";
    QString ip = "";
    quint64 mac = 0;
    QString status = "unreachable";
    QString macStr = "";
    bool vhPortOpened = false;
    bool atemPortOpened = false;
    bool cfgPortOpened = false;
    bool serverPortOpened = false;
    bool anotherUdpOpened = false;

    QDomNode childNode = devNode.firstChild();
    while (!childNode.isNull()) {
        QDomElement child = childNode.toElement();
        if (child.tagName() == "status") {
            status = child.attribute("state", "down");
        }
        else if (child.tagName() == "address") {
            if(child.attribute("addrtype") == "ipv4"){
                ip = child.attribute("addr");
            }
            else if(child.attribute("addrtype") == "mac"){
                macStr = child.attribute("addr");
                mac = NetUtils::strToMac(child.attribute("addr"));
                if(name == "" && child.hasAttribute("vendor")){
                    name = child.attribute("vendor", "");
                }
            }
        }
        else if(child.tagName() == "hostnames" && !child.firstChild().isNull()){
            name = child.firstChild().toElement().attribute("name", name);
        }
        else if(child.tagName() == "ports"){
            QDomNodeList ports = child.elementsByTagName("port");
            int nbr = ports.count();
            for(int i = 0; i < nbr; i++){
                QDomElement port = ports.at(i).toElement();
                if(port.hasAttribute("protocol") && (port.attribute("protocol") == "udp") &&
                        port.hasAttribute("portid") && port.attribute("portid") == STRINGIFY(ATEM_PORT) && port.elementsByTagName("state").count() > 0){
                    QDomElement state = port.elementsByTagName("state").at(0).toElement();
                    if(state.hasAttribute("state") && state.attribute("state").contains("open")){
                        atemPortOpened = true;
                    }
                }
                if(port.hasAttribute("protocol") && (port.attribute("protocol") == "udp") &&
                        port.hasAttribute("portid") && port.attribute("portid") == STRINGIFY(CHAPI_TCP_PORT) && port.elementsByTagName("state").count() > 0){
                    QDomElement state = port.elementsByTagName("state").at(0).toElement();
                    if(state.hasAttribute("state") && state.attribute("state").contains("open")){
                        anotherUdpOpened = true;
                    }
                }
                if(port.hasAttribute("protocol") && (port.attribute("protocol") == "tcp") &&
                        port.hasAttribute("portid") && port.attribute("portid") == STRINGIFY(VIDEOHUB_PORT) && port.elementsByTagName("state").count() > 0){
                    QDomElement state = port.elementsByTagName("state").at(0).toElement();
                    if(state.hasAttribute("state") && state.attribute("state").contains("open")){
                        vhPortOpened = true;
                    }
                }
                if(port.hasAttribute("protocol") && (port.attribute("protocol") == "tcp") &&
                        port.hasAttribute("portid") && port.attribute("portid") == STRINGIFY(CHAPI_TCP_PORT) && port.elementsByTagName("state").count() > 0){
                    QDomElement state = port.elementsByTagName("state").at(0).toElement();
                    if(state.hasAttribute("state") && state.attribute("state").contains("open")){
                        cfgPortOpened = true;
                    }
                }
                if(port.hasAttribute("protocol") && (port.attribute("protocol") == "udp") &&
                        port.hasAttribute("portid") && port.attribute("portid") == STRINGIFY(CHAPISERVER_UDP_PORT) && port.elementsByTagName("state").count() > 0){
                    QDomElement state = port.elementsByTagName("state").at(0).toElement();
                    if(state.hasAttribute("state") && state.attribute("state").contains("open")){
                        cfgPortOpened = true;
                    }
                }
            }
        }
        childNode = child.nextSibling();
    }

    if(mac != 0){ // ignore local interface
        DeviceInfo computer;
        computer.mac = mac;
        computer.ip = ip;
        computer.name = name;
        computer.status = Device::Located;
        computer.type = guessType(cfgPortOpened, vhPortOpened, atemPortOpened, serverPortOpened, anotherUdpOpened, ip);
        emit deviceDetected(computer);
    }
}

Device::DeviceType DeviceScanner::guessType(bool cfgPort, bool vhPort, bool atemPort, bool serverPort, bool anotherUdpOpened, const QString &ip) {
    if(serverPort && !cfgPort && !atemPort && !vhPort && !anotherUdpOpened){
        return Device::ChapiServer;
    }
    if(cfgPort && !serverPort && !atemPort && !vhPort && !anotherUdpOpened){
        return Device::ChapiDev;
    }
    if(vhPort && !serverPort && !cfgPort && !atemPort && !anotherUdpOpened){
        return Device::VideoHub;
    }
    if(atemPort && !serverPort && !cfgPort && !vhPort && !anotherUdpOpened) {
        return Device::Atem;
    }
    else{
        foreach(QHostAddress gateway, NetUtils::getGateways()) {
            if(gateway.toString() == ip){
                return Device::Router;
            }
        }
        return Device::UnknownDevice;
    }
}

void DeviceScanner::helloReceived() {
    while (_udpSocket.hasPendingDatagrams()) {
        QByteArray data;
        qint64 dataSize = _udpSocket.pendingDatagramSize();
        data.fill(0, dataSize+1);
        _udpSocket.readDatagram(data.data(), dataSize);

        QString msg = QString::fromLatin1(data);

        QString macStr;
        QString ip;
        QString version;
        QString kind;
        Device::DeviceType type = Device::UnknownDevice;

        QRegExp regex("^HELLO ([a-zA-Z_]+) ([0-9]+\\.[0-9]+) ((?:[A-fa-f0-9]{2}:){5}[A-fa-f0-9]{2}) ((?:[0-9]{1,3}\\.){3}[0-9]{1,3})\n$");
        if(regex.exactMatch(msg)){
            kind = regex.cap(1);
            version = regex.cap(2);
            macStr = regex.cap(3);
            ip = regex.cap(4);

            if(kind == DEV_CHAPI_DEVICE){
                type = Device::ChapiDev;
            }
            else if(kind == DEV_CHAPI_SERVER) {
                type = Device::ChapiServer;
            }

            DeviceInfo computer;
            computer.mac = NetUtils::strToMac(macStr);
            computer.ip = ip;
            computer.name = "";
            computer.status = Device::Located;
            computer.type = type;
            emit deviceDetected(computer);
        }
    }
}
