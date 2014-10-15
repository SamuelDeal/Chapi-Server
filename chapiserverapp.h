#ifndef CHAPISERVERAPP_H
#define CHAPISERVERAPP_H

#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>


class AppTrayView;
class SyslogModel;
class DeviceList;
class SysLogger;

class ChapiServerApp : public QApplication
{
    Q_OBJECT
public:
    explicit ChapiServerApp(int &argc, char **argv);
    ~ChapiServerApp();

    void start();
    void launch();
    bool isQuitting() const;

private:
    static bool _quitting;

    void openMainWindow();
    void openSyslogWindow();

    QLocalServer _localServer;
    QLocalSocket _localSocket;
    QTimer *_localCnxTimeout;
    QWidget *_mainWindow;
    QWidget *_syslogWindow;
    DeviceList *_devList;
    SysLogger *_syslog;
    SyslogModel *_syslogModel;
    AppTrayView *_trayView;

signals:

public slots:
    void onNewLocalConnection();
    void onMainWindowShowAsked();
    void onMainWindowHideAsked();
    void onMainWindowClosed();
    void onSyslogWindowShowAsked();
    void onSyslogWindowClosed();
    void onLastWindowClosed();
    void onNmapNeeded();
    void onRootNeeded();
    void onAboutAsked();
    void onExitAsked();
    void onPreviousInstanceDetected();
    void onLocalSocketError(QLocalSocket::LocalSocketError);
    void onLocalSocketTimeout();
};

#endif // CHAPISERVERAPP_H
