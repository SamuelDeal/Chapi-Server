#include "chapiserverapp.h"

#include <QLocalSocket>
#include <QTranslator>
#include <QLibraryInfo>
#include <QAbstractEventDispatcher>
#include <QAbstractNativeEventFilter>
#include <functional>
#include <QDesktopServices>
#include <QUrl>
#include <QThreadPool>

#include "rsc_strings.h"
#include "models/devicelist.h"
#include "models/syslogger.h"
#include "models/syslogmodel.h"
#include "views/apptrayview.h"
#include "views/nmappathview.h"
#include "views/mainview.h"
#include "views/syslogview.h"
#include "utils/sighandler.h"
#include "views/versionselectorview.h"

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

    MSG* msg = (MSG*)(message);
    if((msg->message == WM_CLOSE) && (msg->hwnd == 0)){
        ChapiServerApp *app = static_cast<ChapiServerApp*>(ChapiServerApp::instance());
        QMetaObject::invokeMethod(app, "onExitAsked", Qt::QueuedConnection);
        return true;
    }

    return false;
}
#endif // _WIN32

bool ChapiServerApp::_quitting = false;

ChapiServerApp::ChapiServerApp(int &argc, char **argv) :
    QApplication(argc, argv), _devList(), _syslog(_devList), _versionList(), _localServer(this), _localSocket(this)
{
    QApplication::setApplicationName("chapi_server");
    _mainWindow = NULL;
    _syslogWindow = NULL;
    _trayView = NULL;

    connect(&_localSocket, SIGNAL(connected()), this, SLOT(onPreviousInstanceDetected()));
    connect(&_localSocket, SIGNAL(error(QLocalSocket::LocalSocketError)), this, SLOT(onLocalSocketError(QLocalSocket::LocalSocketError)));
    _localCnxTimeout = new QTimer(this);
    connect(_localCnxTimeout, SIGNAL(timeout()), this, SLOT(onLocalSocketTimeout()));
    _localCnxTimeout->start(500);
    _localSocket.connectToServer("__chapi_server");

    setWindowIcon(QIcon(":/icons/imgs/chapi.png"));
}

ChapiServerApp::~ChapiServerApp() {
    if(_trayView != NULL){
        delete _trayView;
    }
}

VersionList &ChapiServerApp::versionList() {
    return _versionList;
}

void ChapiServerApp::onPreviousInstanceDetected() {
    _localCnxTimeout->stop();
    _localSocket.write("PING\n");
    _localSocket.flush();
    _localSocket.close();
    _quitting = true;
    quit();
}

bool ChapiServerApp::isQuitting() const {
    return _quitting;
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

    setWindowIcon(QIcon(":/icons/imgs/chapi.png"));

    QLocale::setDefault(QLocale(QLocale::French, QLocale::France));
    QTranslator *translator = new QTranslator();
    QString lang = QLocale::system().name();
    lang.truncate(2);
    if(translator->load("qt_"+lang+".qm", ":/i18n")){
        installTranslator(translator);
    };

    connect(this, SIGNAL(lastWindowClosed()), this, SLOT(onLastWindowClosed()));
    QApplication::setQuitOnLastWindowClosed(false);

    SigHandler::get().addCallback([&]() -> bool{
        ChapiServerApp *app = static_cast<ChapiServerApp*>(ChapiServerApp::instance());
        QMetaObject::invokeMethod(app, "onExitAsked", Qt::QueuedConnection);
        return true;
    });

#ifdef _WIN32
    QCoreApplication::eventDispatcher()->installNativeEventFilter(new WindowsEventFilter());
#endif
    openMainWindow();
    QMetaObject::invokeMethod(this, "startModels", Qt::QueuedConnection);
}

void ChapiServerApp::startModels() {
    connect(&_devList, SIGNAL(needNmap()), this, SLOT(onNmapNeeded()));
    connect(&_devList, SIGNAL(needRoot()), this, SLOT(onRootNeeded()));
    connect(&_versionList, SIGNAL(newVersionAvailable(Version*)), this, SLOT(onNewVersionAvailable(Version*)));
    connect(&_versionList, SIGNAL(newVersionsAvailables(quint32)), this, SLOT(onNewVersionAvailable(quint32)));

    _devList.load();
    _versionList.start();
    _devList.startScans();

    _trayView = new AppTrayView();
    _trayView->onMainWindowVisibilityChanged(_mainWindow != NULL);
    connect(_trayView, SIGNAL(mainWindowShowCmd()), this, SLOT(onMainWindowShowAsked()));
    connect(_trayView, SIGNAL(mainWindowHideCmd()), this, SLOT(onMainWindowHideAsked()));
    connect(_trayView, SIGNAL(exitCmd()), this, SLOT(onExitAsked()));
}

void ChapiServerApp::onMainWindowShowAsked() {
    openMainWindow();
}

void ChapiServerApp::onMainWindowHideAsked() {
    if(_mainWindow == NULL){
        return;
    }
    _mainWindow->hideInTray();
}

void ChapiServerApp::openMainWindow() {
    if(_mainWindow == NULL){
        _mainWindow = new MainView(_devList, _syslogStatus, _versionList);
        _mainWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(_mainWindow, SIGNAL(destroyed()), this, SLOT(onMainWindowClosed()));
        connect(_mainWindow, SIGNAL(exitCmd()), this, SLOT(onExitAsked()));
        connect(_mainWindow, SIGNAL(syslogWindowShowCmd()), this, SLOT(onSyslogWindowShowAsked()));
        connect(_mainWindow, SIGNAL(aboutCmd()), this, SLOT(onAboutAsked()));
        connect(_mainWindow, SIGNAL(chapiHelpCmd()), this, SLOT(onChapiHelpAsked()));
        connect(_mainWindow, SIGNAL(chapiServerHelpCmd()), this, SLOT(onChapiServerHelpAsked()));
        connect(_mainWindow, SIGNAL(chapiUpdateAsked(Device*)), this, SLOT(onChapiUpdateAsked(Device*)));
    }
    _mainWindow->show();
    setActiveWindow(_mainWindow);
    _mainWindow->activateWindow();
    _mainWindow->setFocus();
    _mainWindow->raise();
    if(_trayView){
        _trayView->onMainWindowVisibilityChanged(true);
    }
}

void ChapiServerApp::onMainWindowClosed() {
    _mainWindow = NULL;
    _trayView->onMainWindowVisibilityChanged(false);
}

void ChapiServerApp::onSyslogWindowShowAsked() {
    openSyslogWindow();
}

void ChapiServerApp::openSyslogWindow() {
    if(_syslogWindow == NULL){
        _syslogWindow = new SyslogView(_syslog.model(), _devList);
        _syslogWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(_syslogWindow, SIGNAL(destroyed()), this, SLOT(onSyslogWindowClosed()));
    }
    _syslogWindow->show();
    _syslogWindow->setFocus();
    _syslogStatus.setDisplayed();
}

void ChapiServerApp::onSyslogWindowClosed() {
    _syslogWindow = NULL;
    _syslogStatus.setHidden();
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
        _devList.setNmapPath(nmapView.newPath());
    }
}


void ChapiServerApp::onRootNeeded() {
    QMessageBox::warning(_mainWindow, tr("Besoin des droits d'administration"), tr("Vous n'avez pas les droits d'administration.\n"
        "Ceux-ci sont requis afin de scanner l'environnement afin de détecter les machines proches.\n"
        "Pour le bon déroulement de cette application, merci de l'éxécuter avec les droits root"), QMessageBox::Ok);
    QApplication::quit();
}

void ChapiServerApp::onAboutAsked() {
    QMessageBox::about(_mainWindow, "A propos de Chapi Serveur", "Chapi serveur est un logiciel prévu pour configurer un Chapi.\n"
                       "Ce logiciel développé sous license GPLv3 ou supérieur.\n"
                       "Le code est disponible sur Github");
}

void ChapiServerApp::onChapiHelpAsked() {
    QDesktopServices::openUrl(QUrl(RscStrings::chapi_wiki_url));
}

void ChapiServerApp::onChapiServerHelpAsked() {
    QDesktopServices::openUrl(QUrl(RscStrings::chapiserver_wiki_url));
}

void ChapiServerApp::onExitAsked(){
    if(_trayView != NULL) {
        disconnect(_trayView, SIGNAL(mainWindowHideCmd()), this, SLOT(onMainWindowHideAsked()));
    }
    _devList.save();
    _versionList.save();
    closeAllWindows();
    QThreadPool::globalInstance()->waitForDone(5000);
    quit();
}

void ChapiServerApp::onNewVersionAvailable(Version *newVersion){
    qDebug() << "NEW VERSION CATCHED !!!";
    QMessageBox::information(_mainWindow, "Nouvelle version disponible",
       "Une nouvelle version de Chapi est disponible: la version "+newVersion->name()+".\n"+
       "Pour mettre à jour un Chapi, veuillez cliquer sur la flèche bleu"+
        "correspondant au boitier que vous désirez mettre à jour.");
}

void ChapiServerApp::onNewVersionsAvailables(quint32 nbrNewVersion) {
    qDebug() << "NEW VERSION CATCHED !!!";
    QMessageBox::information(_mainWindow, "Nouvelles versions disponibles",
       QString::number(nbrNewVersion) + " nouvelles versions de Chapi sont disponibles.\n"+
       "Pour mettre à jour un Chapi, veuillez cliquer sur la flèche bleu"+
        "correspondant au boitier que vous désirez mettre à jour.");
}

void ChapiServerApp::onChapiUpdateAsked(Device* dev) {
    Version* result = VersionSelectorView::select(dev->version(), _versionList);
    if(!result){
        return;
    }
}
