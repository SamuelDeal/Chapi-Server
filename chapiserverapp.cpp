#include "chapiserverapp.h"

#include <qlocalsocket.h>
#include <QTranslator>
#include <QLibraryInfo>
#include <QAbstractEventDispatcher>
#include <QAbstractNativeEventFilter>
#include <functional>

#include "views/nmappathview.h"
#include "views/mainview.h"
#include "utils/sighandler.h"


//TODO: rename hatuin to Chapi : Controller de Hub Audio-vidéo
//Par Informatique
//Presque Indolore
//Provenant d'Insomnies
//Plutot Inovant/Ingénieux
//Prétenduement instinctif
//prodigieusement itinérant

//TODO check for translations :Qt bug?

#ifdef _WIN32
class WindowsEventFilter : public QAbstractNativeEventFilter {
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
};

bool WindowsEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result) {
    Q_UNUSED(eventType);
    Q_UNUSED(result);

    //TODO: ask on the Qt mailing list if it's not a bug
    //TODO: check for event of kill windows
    MSG* msg = (MSG*)(message);
    if((msg->message == WM_CLOSE) && (msg->hwnd == 0)){
        ChapiServerApp *app = static_cast<ChapiServerApp*>(ChapiServerApp::instance());
        QMetaObject::invokeMethod(app, "onExitAsked", Qt::QueuedConnection);
        return true;
    }

    return false;
}
#endif // _WIN32

ChapiServerApp::ChapiServerApp(int &argc, char **argv) :
    QApplication(argc, argv), _localServer(this), _localSocket(this)
{
    QApplication::setApplicationName("chapi_server");
    _mainWindow = NULL;
    _devList = NULL;
    _trayView = NULL;

    connect(&_localSocket, SIGNAL(connected()), this, SLOT(onPreviousInstanceDetected()));
    connect(&_localSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(onLocalSocketError(QLocalSocket::LocalSocketError)));
    _localCnxTimeout = new QTimer(this);
    connect(_localCnxTimeout, SIGNAL(timeout()), this, SLOT(onLocalSocketTimeout()));
    _localCnxTimeout->start(1000);
    _localSocket.connectToServer("__chapi_server");

    setWindowIcon(QIcon(":/icons/chapi.png"));
}

void ChapiServerApp::onPreviousInstanceDetected() {
    _localCnxTimeout->stop();
    _localSocket.write("PING\n");
    _localSocket.flush();
    _localSocket.close();
    quit();
}

void ChapiServerApp::onLocalSocketTimeout() {
    onLocalSocketError(QLocalSocket::SocketTimeoutError);
}

void ChapiServerApp::onLocalSocketError(QLocalSocket::LocalSocketError err) {
    Q_UNUSED(err);

    _localCnxTimeout->stop();
    disconnect(&_localSocket, SIGNAL(connected()), this, SLOT(onPreviousInstanceDetected()));
    disconnect(&_localSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(onLocalSocketError(QLocalSocket::LocalSocketError)));
    _localSocket.abort();
    _localSocket.close();
    launch();
}

void ChapiServerApp::launch() {
    connect(&_localServer, SIGNAL(newConnection()), this, SLOT(onNewLocalConnection()));

    _localServer.listen("__chapi_server");

    setWindowIcon(QIcon(":/icons/chapi.png"));

    QLocale::setDefault(QLocale(QLocale::French, QLocale::France));
    QTranslator *translator = new QTranslator();
    QString lang = QLocale::system().name();
    lang.truncate(2);
    if(translator->load("qt_"+lang+".qm", ":/i18n")){
        installTranslator(translator);
    };

    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(onLastWindowClosed()));
    QApplication::setQuitOnLastWindowClosed(false);

    _devList = new DeviceList;
    connect(_devList, SIGNAL(needNmap()), this, SLOT(onNmapNeeded()));
    connect(_devList, SIGNAL(needRoot()), this, SLOT(onRootNeeded()));
    _devList->load();

    _trayView = new AppTrayView();
    connect(_trayView, SIGNAL(mainWindowShowCmd()), this, SLOT(onMainWindowShowAsked()));
    connect(_trayView, SIGNAL(mainWindowHideCmd()), this, SLOT(onMainWindowHideAsked()));
    connect(_trayView, SIGNAL(aboutCmd()), this, SLOT(onAboutAsked()));
    connect(_trayView, SIGNAL(exitCmd()), this, SLOT(onExitAsked()));

    SigHandler::get().addCallback([&]() -> bool{
        ChapiServerApp *app = static_cast<ChapiServerApp*>(ChapiServerApp::instance());
        QMetaObject::invokeMethod(app, "onExitAsked", Qt::QueuedConnection);
        return true;
    });

#ifdef _WIN32
    QCoreApplication::eventDispatcher()->installNativeEventFilter(new WindowsEventFilter());
#endif
    openMainWindow();
}

ChapiServerApp::~ChapiServerApp() {
    if(_devList != NULL){
        _devList->save();
        delete _devList;
    }
    if(_trayView != NULL){
        delete _trayView;
    }
}

void ChapiServerApp::onMainWindowShowAsked() {
    openMainWindow();
}

void ChapiServerApp::onMainWindowHideAsked() {
    if(_mainWindow == NULL){
        return;
    }
    _mainWindow->close();
}

void ChapiServerApp::openMainWindow() {
    if(_mainWindow == NULL){
        _mainWindow = new MainView(_devList);
        _mainWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(_mainWindow, SIGNAL(destroyed()), this, SLOT(onMainWindowClosed()));
    }
    _mainWindow->show();
    setActiveWindow(_mainWindow);
    _mainWindow->setFocus();
    _trayView->onMainWindowVisibilityChanged(true);
}

void ChapiServerApp::onMainWindowClosed() {
    _mainWindow = NULL;
    _trayView->onMainWindowVisibilityChanged(false);
}

void ChapiServerApp::onNewLocalConnection() {
    QLocalSocket *localSocket = _localServer.nextPendingConnection();
    connect(localSocket, SIGNAL(disconnected()), localSocket, SLOT(deleteLater()));

    while (localSocket->bytesAvailable() == 0) {
        localSocket->waitForReadyRead();
    }
    localSocket->disconnectFromServer();
    openMainWindow();
    _mainWindow->setFocus();
}

void ChapiServerApp::onLastWindowClosed() {
    _trayView->displayTooltip();
}

void ChapiServerApp::onNmapNeeded() {
    NmapPathView nmapView(_mainWindow);
    nmapView.exec();

    if(nmapView.isCanceled()){
        QApplication::quit();
    }
    else {
        _devList->setNmapPath(nmapView.newPath());
    }
}


void ChapiServerApp::onRootNeeded() {
    QMessageBox::warning(_mainWindow, tr("Besoin des droits d'administration"), tr("Vous n'avez pas les droits d'administration.\n"
        "Ceux-ci sont requis afin de scanner l'environnement afin de détecter les machines proches.\n"
        "Pour le bon déroulement de cette application, merci de l'éxécuter avec les droits root"), QMessageBox::Ok);
    QApplication::quit();
}

void ChapiServerApp::onAboutAsked() {
    //TODO: description du about
    QMessageBox::about(_mainWindow, "A propos de Chapi Serveur", "Description\nsur\nplusieurs\nlignes\n<a href='http://www.google.com'>test lien</a>");
}

void ChapiServerApp::onExitAsked(){
    closeAllWindows();
    quit();
}
