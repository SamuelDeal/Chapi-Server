#ifndef CHAPISERVERAPP_H
#define CHAPISERVERAPP_H

#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>

#include "models/devicelist.h"
#include "models/syslogger.h"
#include "models/versionlist.h"
#include "models/syslogwindowstatus.h"

class AppTrayView;
class SyslogModel;
class DeviceList;
class SysLogger;
class VersionList;
class MainView;

class ChapiServerApp : public QApplication
{
    Q_OBJECT
public:
    explicit ChapiServerApp(int &argc, char **argv);
    ~ChapiServerApp();

    void launch();
    bool isQuitting() const;
    VersionList& versionList();

private:
    static bool _quitting;

    void openMainWindow();
    void openSyslogWindow();

    DeviceList _devList;
    SysLogger _syslog;
    SyslogWindowStatus _syslogStatus;
    VersionList _versionList;

    QLocalServer _localServer;
    QLocalSocket _localSocket;
    QTimer *_localCnxTimeout;
    MainView *_mainWindow;
    QWidget *_syslogWindow;
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
    void onChapiHelpAsked();
    void onChapiServerHelpAsked();
    void onAboutAsked();
    void onExitAsked();
    void onPreviousInstanceDetected();
    void onLocalSocketError(QLocalSocket::LocalSocketError);
    void onLocalSocketTimeout();
    void startModels();
    void onNewVersionAvailable(Version*);
    void onNewVersionsAvailables(quint32);
    void onChapiUpdateAsked(Device*);
};

#endif // CHAPISERVERAPP_H
