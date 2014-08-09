#include "connecteddevice.h"

#include <QSettings>

ConnectedDevice::ConnectedDevice(const Device &dev) :
    Device(dev), _socket(this) {
    connect(&_socket, SIGNAL(readyRead()), this, SLOT(onData()));
    connect(&_socket, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&_socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError(QAbstractSocket::SocketError)));
    connect(&_pingTimer, SIGNAL(timeout()), this, SLOT(onPing()));
    connect(&_reconnectTimer, SIGNAL(timeout()), this, SLOT(onReconnectDelayExpired()));

    if(!_ip.isEmpty()){
        qDebug() << "constructor ip not empty";
        QMetaObject::invokeMethod(this, "connectToDevice", Qt::QueuedConnection);
    }
    else {
        qDebug() << "empty ip constructor";
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
    qDebug() << "ip set to " << ip;
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
    qDebug() << "connectToDevice";
    _lastIp = _ip;
    makeConnection();
    setStatus(Device::Connecting);
}

void ConnectedDevice::makeConnection(){
    qDebug() << "starting connection " << _lastIp << " on port " << port() << " " << (quint64) this;
    _socket.close();
    _socket.abort();
    pausePing();
    _reconnectTimer.stop();
    _socket.connectToHost(_lastIp, port());
}

void ConnectedDevice::onReconnectDelayExpired() {
    qDebug() << "onReconnectDelayExpired";
    makeConnection();
}

void ConnectedDevice::onData() {
    parseInput();
}

void ConnectedDevice::onError(QAbstractSocket::SocketError error) {
    qDebug() << "error: " << error << _socket.errorString();
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
    setStatus(Device::Connected);
    resumePing();
}

void ConnectedDevice::onPing() {
    if(_pingSent > pingLostTolerance()) {
        qDebug() << "ping failed" << _pingSent << "times, disconnect";
        closeCnx(true);
    }
    else {
        ping();
        ++_pingSent;
    }
}

void ConnectedDevice::loadSpecific(QSettings &settings) {
    Device::loadSpecific(settings);
}

void ConnectedDevice::saveSpecific(QSettings &settings) {
    Device::saveSpecific(settings);
    if(!_lastIp.isEmpty()){
        settings.setValue("last_ip", _lastIp);
    }
}
