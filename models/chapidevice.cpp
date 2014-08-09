#include "chapidevice.h"

#include "../const.h"
#include "../utils/netutils.h"
#include "devicelist.h"
#include "targetabledevice.h"

const quint8 ChapiDevice::NO_SOURCE;

bool operator==(const NetworkConfig& lhs, const NetworkConfig& rhs) {
    return lhs.useDHCP  == rhs.useDHCP &&
           lhs.ip       == rhs.ip &&
           lhs.netmask  == rhs.netmask &&
           lhs.gateway  == rhs.gateway;
}

bool operator!=(const NetworkConfig& lhs, const NetworkConfig& rhs) {
    return !(lhs== rhs);
}

ChapiDevice::ChapiDevice(const Device &dev, DeviceList *devList) :
    ConnectedDevice(dev), _protocol(&_socket) {
    _netCfg.useDHCP = true;
    _nbrBtns = 0;
    _devList = devList;
    _target = NULL;
    connect(&_protocol, SIGNAL(onCommandReceived(NlCommand)), this, SLOT(onCommandReceived(NlCommand)));
    connect(&_protocol, SIGNAL(onCommandFailed(NlCommand)), this, SLOT(onCommandFailed(NlCommand)));
}

ChapiDevice::~ChapiDevice(){
}

quint16 ChapiDevice::port() const {
    return CHAPI_TCP_PORT;
}

int ChapiDevice::pingDelay() const {
    return 500;
}

quint32 ChapiDevice::pingLostTolerance() const {
    return 2;
}

int ChapiDevice::reconnectDelay() const {
    return 2000;
}

bool ChapiDevice::isConfigurable() const {
    return true;
}

bool ChapiDevice::isConfigured() const {
    return _configSet;
}

bool ChapiDevice::isIdentifiable() const {
    return true;
}

Device::DeviceSimpeStatus ChapiDevice::simpleStatus() const {
    if(_status == Device::Ready){
        return Device::Green;
    }
    if((_status == Device::Unreachable) || (_status == Device::Located)) {
        return Device::Red;
    }
    return Device::Yellow;
}

void ChapiDevice::blink() {
    _protocol.sendCommand("BLINK");
}

void ChapiDevice::restart() {
    _protocol.sendCommand("RESTART");
    emit resetIp();
    closeCnx(false);
}

void ChapiDevice::parseInput(){
    _protocol.read();
}

void ChapiDevice::onCommandFailed(NlCommand cmd){
    qDebug() << "command " << cmd.command << " failed";
    foreach(QString line, cmd.lines){
        qDebug() << line;
    }
}

void ChapiDevice::saveConfig(NetworkConfig &netConfig) {
    setStatus(Device::ApplyingConfig);
    pausePing();
    _protocol.sendCommand("CONFIG_BEGIN");
    _protocol.sendCommand("TARGET_MAC", NetUtils::macToStr(_targetMac));
    _protocol.sendCommand("TARGET_IP", _targetIp);
    _protocol.sendCommand("OUTPUTS", "0 "+QString::number(_outputIndex));
    QStringList inputLines;
    for(quint8 i = 0; i < nbrButtons(); i++){
        inputLines.push_back(QString::number(i)+" "+QString::number(_inputIndexes[i]));
    }
    _protocol.sendCommand("INPUTS", inputLines);

    if(netConfig != _netCfg){
        QStringList netLines;
        netLines.push_back("dhcp "+netConfig.useDHCP ? "1" : "0");
        if(!netConfig.ip.isEmpty()){
            netLines.push_back("ip "+netConfig.ip);
            if(!netConfig.netmask.isEmpty()){
                netLines.push_back("mask "+netConfig.netmask);
            }
            if(!netConfig.gateway.isEmpty()){
                netLines.push_back("gateway "+netConfig.gateway);
            }
        }
        _protocol.sendCommand("NETWORK", netLines);
    }

    _protocol.sendCommand("CONFIG_END");
    updateDeviceStatus();
    resumePing();
}

void ChapiDevice::setTarget(TargetableDevice *device) {
    if(_target != NULL){
        disconnect(_target, SIGNAL(changed()), this, SLOT(onTargetChanged()));
    }
    _targetMac = device->mac();
    _target = device;
    if(_target != NULL){
        _targetIp = _target->lastKnownIp();
        connect(_target, SIGNAL(changed()), this, SLOT(onTargetChanged()));
    }
}

void ChapiDevice::onCommandReceived(NlCommand cmd){
    if(cmd.command == "PONG") {
        qDebug() << "pong received";
        _pingSent = 0;
    }
    else if(cmd.command == "CONFIG_BEGIN"){
        pausePing();
        setStatus(Device::ReadingConfig);
    }
    else if(cmd.command == "CONFIG_END") {
        resumePing();
        updateDeviceStatus();
    }
    else if(cmd.command == "CONFIG_SET"){
        _configSet = (cmd.lines.length() > 0) && (cmd.lines[0] == "1");
    }
    else if(cmd.command == "NBR_BUTTONS"){ //Warning: should always be sent before input or output
        bool ok;
        int value = (cmd.lines.length() == 0) ? -1 : cmd.lines[0].toInt(&ok);
        _nbrBtns = ok ? value : -1;
    }
    else if(cmd.command == "VERSION"){
        if((cmd.lines.length() > 0)){
            setVersion(cmd.lines[0]);
        }
    }
    else if(cmd.command == "TARGET_TYPE"){ //Warning: should always be sent before input or output
        //we don't care
    }
    else if(cmd.command == "TARGET_IP"){ //Warning: should always be sent before TARGET_MAC
        if((cmd.lines.length() > 0)){
            if((_target == NULL) || (_target->ip().isEmpty())){
                _targetIp = cmd.lines[0];
            }
        }
    }
    else if(cmd.command == "TARGET_MAC"){
        if((cmd.lines.length() > 0)){
            setTarget(dynamic_cast<TargetableDevice*>(_devList->deviceByMac(NetUtils::strToMac(cmd.lines[0]))));
        }
    }
    else if(cmd.command == "OUTPUTS"){
        QRegExp regex("^0 +([0-9]+) *$");
        foreach(QString line, cmd.lines){
            if(regex.exactMatch(line)){
                _outputIndex = regex.cap(1).toUShort();
                break;
            }
        }
    }
    else if(cmd.command == "INPUTS"){
        for(quint8 i = 0; i < _nbrBtns; i++) {
            _inputIndexes[i] = NO_SOURCE;
        }
        QRegExp regex("^([0-9]+) +([0-9]+) *$");
        foreach(QString line, cmd.lines){
            if(regex.exactMatch(line)){
                _inputIndexes[regex.cap(1).toUShort()] = regex.cap(2).toUShort();
            }
        }
    }
    else if(cmd.command == "NETWORK") {
        QRegExp regex("^([a-z]+) +([^ ]+) *$");
        foreach(QString line, cmd.lines){
            if(!regex.exactMatch(line)){
                continue;
            }
            if(regex.cap(1) == "dhcp") {
                _netCfg.useDHCP = regex.cap(2) == "1";
            }
            else if(regex.cap(1) == "ip") {
                _netCfg.ip = regex.cap(2);
            }
            else if(regex.cap(1) == "mask") {
                _netCfg.netmask = regex.cap(2);
            }
            else if(regex.cap(1) == "gateway") {
                _netCfg.gateway = regex.cap(2);
            }
        }
    }
    else {
        qDebug() << "unknown command " << cmd.command;
    }
}

NetworkConfig ChapiDevice::networkConfig() const {
    return _netCfg;
}

quint8 ChapiDevice::nbrButtons() const {
    return _nbrBtns;
}

void ChapiDevice::ping() {
    _protocol.sendCommand("PING");
}

void ChapiDevice::updateDeviceStatus(){
    if(!_configSet) {
        setStatus(Device::Unconfigured);
    }
    else {
        switch(_cnxStatus){
            case ChapiDevice::Connected:
                setStatus(Device::Ready);
                break;

            case ChapiDevice::Connecting:
                setStatus(Device::ConnectingToHub);
                break;

            default:
            case ChapiDevice::Unreachable:
                setStatus(Device::HubUnreachable);
                break;
        }
    }
}
quint8 ChapiDevice::outputIndex() const {
    return _outputIndex;
}

void ChapiDevice::setOutputIndex(quint8 outputIndex) {
    _outputIndex = outputIndex;
}

quint8 ChapiDevice::inputIndex(quint8 btnIndex) const {
    return _inputIndexes[btnIndex];
}

void ChapiDevice::setInputIndex(quint8 btnIndex, quint8 inputIndex) {
    _inputIndexes[btnIndex] = inputIndex;
}

QString ChapiDevice::targetIp() const {
    return _targetIp;
}

quint64 ChapiDevice::targetMac() const {
    return _targetMac;
}

void ChapiDevice::onTargetChanged() {
    if((_target != NULL) && isConfigurableNow() && (_target->lastKnownIp() != _targetIp) && !_target->lastKnownIp().isEmpty()){
        _targetIp = _target->lastKnownIp();
        _protocol.sendCommand("TARGET_IP", _targetIp);
    }
}
