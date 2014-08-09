#include "connecteddevice.h"

#include <QSettings>
#include <QHostAddress>

ConnectedDevice::ConnectedDevice(const Device &dev) :
    Device(dev), _socket(this) {
    connect(&_socket, SIGNAL(readyRead()), this, SLOT(onData()));
    connect(&_socket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(&_pingTimer, SIGNAL(timeout()), this, SLOT(onPing()));
    connect(&_reconnectTimer, SIGNAL(timeout()), this, SLOT(onReconnectDelayExpired()));

    if(!_ip.isEmpty() && !_lastIp.isEmpty()){
        QMetaObject::invokeMethod(this, "connectToDevice", Qt::QueuedConnection);
    }
}

ConnectedDevice::~ConnectedDevice(){
    _socket.close();
    _socket.abort();
    _reconnectTimer.stop();
    pausePing();
}

void ConnectedDevice::setIp(const QString &ip) {
    if(_ip == ip){
        return;
    }
    _ip = ip;
    _status = ip.isEmpty() ? Device::Unreachable : Device::Located;
    if(ip != ""){
        connectToDevice();
    }
    if(!ip.isEmpty()){
        _lastIp = ip;
    }
    emit changed();
}

QString ConnectedDevice::lastKnownIp() const {
    return _lastIp;
}

void ConnectedDevice::connectToDevice(){
    makeConnection();
    setStatus(_ip.isEmpty() ? Device::Unreachable : Device::Connecting);
}

void ConnectedDevice::makeConnection(){
    _socket.close();
    _socket.abort();
    pausePing();
    _reconnectTimer.stop();
    _socket.connectToHost(_ip.isEmpty() ? _lastIp : _ip, port());
}

void ConnectedDevice::onReconnectDelayExpired() {
    makeConnection();
}

void ConnectedDevice::onData() {
    parseInput();
}

void ConnectedDevice::onError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error);
    closeCnx(true);
}

void ConnectedDevice::closeCnx(bool reconnect) {
    _reconnectTimer.stop();
    pausePing();
    _socket.close();
    _socket.abort();
    setStatus(Device::Unreachable);
    if(reconnect){
        _reconnectTimer.setSingleShot(true);
        _reconnectTimer.start(reconnectDelay());
    }
}

void ConnectedDevice::pausePing() {
    _pingTimer.stop();
    _pingSent = 0;
}

void ConnectedDevice::resumePing() {
    _pingSent = 0;
    _pingTimer.start(pingDelay());
}

void ConnectedDevice::onConnected() {
    _ip = _socket.peerAddress().toString();
    _lastIp = _ip;
    setStatus(Device::Connected);
    resumePing();
}

void ConnectedDevice::onPing() {
    if(_pingSent > pingLostTolerance()) {
        closeCnx(true);
    }
    else {
        ping();
        ++_pingSent;
    }
}

void ConnectedDevice::loadSpecific(QSettings &settings) {
    Device::loadSpecific(settings);
    QString lastKnownIp = settings.value("last_ip", QVariant("")).toString();
    if(lastKnownIp == _lastIp){
        return;
    }
    _lastIp = lastKnownIp;
    if(!_lastIp.isEmpty() && _ip.isEmpty()){
        connectToDevice();
    }
}

void ConnectedDevice::saveSpecific(QSettings &settings) {
    Device::saveSpecific(settings);
    if(!_lastIp.isEmpty()){
        settings.setValue("last_ip", _lastIp);
    }
}
