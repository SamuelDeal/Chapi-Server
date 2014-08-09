#ifndef NLPROTOCOL_H
#define NLPROTOCOL_H

#include <QObject>
#include <QTcpSocket>
#include <queue>

struct NlCommand {
    QString command;
    QStringList lines;
};

class NlProtocol : public QObject
{
    Q_OBJECT
public:
    explicit NlProtocol(QTcpSocket *socket);

    void sendCommand(const QString& cmd, const QStringList &lines);
    void sendCommand(const QString& cmd, const QString &line);
    void sendCommand(const QString& cmd);
    void read();

private:
    void send(const NlCommand& cmd);
    void parseCommand();
    void popSendingQueue();

    QTcpSocket *_socket;
    std::queue<NlCommand> _outQueue;
    bool _reading;
    NlCommand _sendingCommand;
    NlCommand _readingCommand;

signals:
    void onCommandFailed(NlCommand cmd);
    void onCommandReceived(NlCommand cmd);

public slots:

};

#endif // NLPROTOCOL_H
