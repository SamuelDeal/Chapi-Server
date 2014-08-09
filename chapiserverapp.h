#ifndef CHAPISERVERAPP_H
#define CHAPISERVERAPP_H

#include <QApplication>
#include <QLocalServer>
#include <QLocalSocket>

#include "models/devicelist.h"
#include "views/apptrayview.h"

class ChapiServerApp : public QApplication
{
    Q_OBJECT
public:
    explicit ChapiServerApp(int &argc, char **argv);
    ~ChapiServerApp();

    void start();
    void launch();

private:
    void openMainWindow();

    QLocalServer _localServer;
    QLocalSocket _localSocket;
    QTimer *_localCnxTimeout;
    QWidget*_mainWindow;
    DeviceList *_devList;
    AppTrayView *_trayView;

signals:

public slots:
    void onNewLocalConnection();
    void onMainWindowShowAsked();
    void onMainWindowHideAsked();
    void onMainWindowClosed();
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
