#include "nlprotocol.h"

NlProtocol::NlProtocol(QTcpSocket *socket) :
    QObject(NULL)
{
    _socket = socket;
    _reading = false;
}

void NlProtocol::sendCommand(const QString& cmd){
    sendCommand(cmd, QStringList());
}

void NlProtocol::sendCommand(const QString& cmd, const QString &line){
    QStringList lines;
    lines.push_back(line);
    sendCommand(cmd, lines);
}

void NlProtocol::sendCommand(const QString& cmd, const QStringList &lines){
    NlCommand toSend;
    toSend.command = cmd;
    toSend.lines = lines;
    if(_reading){
        _outQueue.push(toSend);
    }
    else{
        send(toSend);
    }
}

void NlProtocol::send(const NlCommand& cmd) {
    _sendingCommand = cmd;
    _socket->write(cmd.command.toLatin1());
    _socket->write(":\n");
    foreach(QString line, cmd.lines){
        _socket->write(line.toLatin1());
        _socket->write("\n");
    }
    _socket->write("\n");
    _socket->flush();
}


void NlProtocol::read() {
    _reading = true;
    while(_socket->canReadLine()){
        _reading = true;
        QByteArray buffer = _socket->readLine();
        QString line(buffer);
        line = line.trimmed();
        if(line.isEmpty()){
            _reading = _socket->bytesAvailable() > 0;
            parseCommand();
        }
        else if(_readingCommand.command.isEmpty()) {
            _readingCommand.command = line;
        }
        else{
            _readingCommand.lines.push_back(line);
        }
    }

    if(_socket->bytesAvailable() > 0){
        _reading = true;
    }
}

void NlProtocol::parseCommand() {
    if(_readingCommand.command.isEmpty()){
        return;
    }
    if(_readingCommand.command == "ACK"){
        popSendingQueue();
    }
    else if(_readingCommand.command == "NAK"){
        emit onCommandFailed(_sendingCommand);
        popSendingQueue();
    }
    else{
        _readingCommand.command.resize(_readingCommand.command.length()-1);
        emit onCommandReceived(_readingCommand);
    }
    _readingCommand.command = "";
    _readingCommand.lines.clear();
}

void NlProtocol::popSendingQueue() {
    if(_reading || _outQueue.empty()){
        return;
    }
    send(_outQueue.front());
    _outQueue.pop();
}
