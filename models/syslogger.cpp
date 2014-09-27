#include "syslogger.h"

#include <QDebug>
#include <QHostAddress>
#include <QUdpSocket>

#include "../const.h"
#include "syslogentry.h"
#include "devicelist.h"
#include "../utils/infint.h"
#include "../utils/netutils.h"

SysLogger *SysLogger::_currentLogger = NULL;

SysLogger::SysLogger(DeviceList *devList) :
    QObject(0), _buffer(), _stream(&_buffer)
{
    _devList = devList;
    _currentLogger = this;
    qInstallMessageHandler(SysLogger::logHandler);
    _udpSocket = new QUdpSocket(this);
    _udpSocket->bind(CHAPISERVER_UDP_PORT, QUdpSocket::ShareAddress);

    connect(_udpSocket, SIGNAL(readyRead()), this, SLOT(onData()));

    qDebug() << "Multicast created";
    qDebug() << NetUtils::strToMac("b8:27:eb:3d:8b:70");


    //QHostAddress groupAddress = QHostAddress("239.255.51.13");
    //_udpSocket->bind(QHostAddress::AnyIPv4, 13015, QUdpSocket::ShareAddress);
    //_udpSocket->joinMulticastGroup(groupAddress);
}

SysLogger::~SysLogger(){
    _currentLogger = NULL;
    delete _udpSocket;
    qInstallMessageHandler(0);
}

DeviceList *SysLogger::devList() const {
    return _devList;
}

void SysLogger::logHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Q_UNUSED(context);
    _currentLogger->log(type, msg);
}

void SysLogger::log(QtMsgType type, const QString &msg) {
    emit onNewEntry(SyslogEntry(type, msg, _devList->currentDevice()));
}

void SysLogger::onData() {
    while(_udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(_udpSocket->pendingDatagramSize());
        _udpSocket->readDatagram(datagram.data(), datagram.size());
        _stream << datagram;
    }
    QString line = _stream.readLine();
    while(!line.isEmpty()){
        QRegExp re("^<(\\d{1,3})>(\\w+)\\s+(\\d+)\\s+(\\d+:\\d+:\\d+)\\s+(\\S+)\\s+(.*)$");
        if(re.indexIn(line) != -1){
            QStringList entryData = re.capturedTexts();
            Device *foundHost = NULL;
            foreach(Device* dev, _devList->devices()){
                if(!dev->isLoggable()){
                    continue;
                }
                if(InfInt(entryData[5].toStdString()) % InfInt(dev->mac()) == InfInt(0)) {
                    foundHost = dev;
                    break;
                }
            }
            emit onNewEntry(SyslogEntry(entryData, foundHost));
        }
        line = _stream.readLine();
    }
}

void SysLogger::onConnected(){
    qDebug() << "Multicast connected";
}

void SysLogger::onDisconnected(){
    qDebug() << "Multicast disconnected";
}

void SysLogger::onError(QAbstractSocket::SocketError error) {
    qDebug() << "Multicast error:" << error;
}

void SysLogger::onHostFound(){
    qDebug() << "Multicast host found";
}

void SysLogger::onStateChanged(QAbstractSocket::SocketState state) {
    qDebug() << "Multicast state changed to: " << state;
}
