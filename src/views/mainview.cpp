#include "mainview.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QFile>
#include <QSettings>
#include <QScrollArea>
#include <QCloseEvent>
#include <QStackedLayout>
#include <QLabel>
#include <QStandardPaths>
#include <QPushButton>
#include <QMenu>


#include "../models/devicelist.h"
#include "../models/syslogwindowstatus.h"
#include "../models/versionlist.h"
#include "deviceview.h"
#include "chapiview.h"
#include "videohubview.h"
#include "atemview.h"

MainView::MainView(DeviceList &devList, SyslogWindowStatus &syslogStatus, VersionList &versionList, QWidget *parent) :
    QMainWindow(parent), _devList(devList), _syslogStatus(syslogStatus), _versionList(versionList)
{
    setWindowTitle(tr("Chapi serveur"));
    _settingsMenu = NULL;

    connect(&_devList, SIGNAL(deviceListChanged()), this, SLOT(onDeviceListChanged()));
    initTabs();

    _stackedLayout = new QStackedLayout();
    _mainPage = new QWidget();
    _stackedLayout->addWidget(_mainPage);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->addWidget(_tabs);
    _mainPage->setLayout(mainLayout);
    _mainPage->updateGeometry();


    setCentralWidget(new QWidget(this));
    centralWidget()->setLayout(_stackedLayout);

    onDeviceListChanged();

    _settingsBtn = new QPushButton(_mainPage);
    _settingsBtn->updateGeometry();
    _settingsBtn->setIcon(QIcon(":/icons/imgs/settings.png"));
    _settingsBtn->setIconSize(QSize(32, 32));
    _settingsBtn->resize(42, 42);

    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);
    resize(settings.value("MainView/size", QSize(450, 300)).toSize());
    move(settings.value("MainView/pos", QPoint(200, 200)).toPoint());
    _exitOnClose = settings.value("MainView/exit_on_close", false).toBool();
    if(settings.value("MainView/maximized", false).toBool()){
        showMaximized();
    }
    else {
        _settingsBtn->move(settings.value("MainView/size", QSize(450, 300)).toSize().width() - _settingsBtn->width() - 5, 5);
    }
    connect(_settingsBtn, SIGNAL(clicked()), this, SLOT(onSettingsClick()));
    connect(&_syslogStatus, SIGNAL(visibilityChanged(bool)), this, SLOT(onSyslogWindowVisibilityChanged(bool)));
    QMetaObject::invokeMethod(this, "onResized", Qt::QueuedConnection);
}

void MainView::initTabs() {
    _tabs = new QTabWidget(this);
    _tabs->setIconSize(QSize(48, 48));
    QFile styleFile(":/styles/tabs.qss");
    styleFile.open(QFile::ReadOnly );
    QString style(styleFile.readAll() );
    _tabs->setStyleSheet(style);

    //Chapi tab init
    QScrollArea* scrollArea = new QScrollArea();
    QWidget *content = new QWidget();
    _chapiLayout = new QVBoxLayout();
    content->setLayout(_chapiLayout);
    _chapiLayout->addStretch(1);
    QLabel *label = new QLabel("Aucun périphérique de ce type n'a été détecté sur le réseau.\n"
                               "\n"
                               "Veuillez vérifier vos branchements et votre configuration réseau.\n"
                               "La détection peut prendre quelques minutes");
    label->setAlignment(Qt::AlignHCenter);
    _chapiLayout->addWidget(label);
    _chapiLayout->addStretch(2);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(content);
    QIcon icon1;
    icon1.addFile(QStringLiteral(":/icons/imgs/chapi.png"), QSize(), QIcon::Normal, QIcon::Off);
    _tabs->addTab(scrollArea, icon1, tr("Chapis"));


    _vhLayout = new QVBoxLayout();
    content = new QWidget();
    content->setLayout(_vhLayout);
    _vhLayout->addStretch(1);
    label = new QLabel("Aucun périphérique de ce type n'a été détecté sur le réseau.\n"
                               "\n"
                               "Veuillez vérifier vos branchements et votre configuration réseau.\n"
                               "La détection peut prendre quelques minutes");
    label->setAlignment(Qt::AlignHCenter);
    _vhLayout->addWidget(label);
    _vhLayout->addStretch(2);
    scrollArea = new QScrollArea();
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(content);
    QIcon icon2;
    icon2.addFile(QStringLiteral(":/icons/imgs/vh.png"), QSize(), QIcon::Normal, QIcon::Off);
    _tabs->addTab(scrollArea, icon2, tr("Video hubs"));


    _atemLayout = new QVBoxLayout();
    content = new QWidget();
    content->setLayout(_atemLayout);
    _atemLayout->addStretch(1);
    label = new QLabel("Aucun périphérique de ce type n'a été détecté sur le réseau.\n"
                               "\n"
                               "Veuillez vérifier vos branchements et votre configuration réseau.\n"
                               "La détection peut prendre quelques minutes");
    label->setAlignment(Qt::AlignHCenter);
    _atemLayout->addWidget(label);
    _atemLayout->addStretch(2);
    scrollArea = new QScrollArea();
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(content);
    QIcon icon3;
    icon3.addFile(QStringLiteral(":/icons/imgs/atem.png"), QSize(), QIcon::Normal, QIcon::Off);
    _tabs->addTab(scrollArea, icon3, tr("Atem switchers"));


    _otherLayout = new QVBoxLayout();
    content = new QWidget();
    content->setLayout(_otherLayout);
    _otherLayout->addStretch(1);
    label = new QLabel("Aucun périphérique de ce type n'a été détecté sur le réseau.\n"
                               "\n"
                               "Veuillez vérifier vos branchements et votre configuration réseau.\n"
                               "La détection peut prendre quelques minutes");
    label->setAlignment(Qt::AlignHCenter);
    _otherLayout->addWidget(label);
    _otherLayout->addStretch(2);
    scrollArea = new QScrollArea();
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(content);
    QIcon icon4;
    icon4.addFile(QStringLiteral(":/icons/imgs/other.png"), QSize(), QIcon::Normal, QIcon::Off);
    _tabs->addTab(scrollArea, icon4, tr("Autres"));

    _devCount.insert(_chapiLayout, 0);
    _devCount.insert(_vhLayout, 0);
    _devCount.insert(_atemLayout, 0);
    _devCount.insert(_otherLayout, 0);

    _tabs->setCurrentIndex(0);
}


void MainView::closeEvent(QCloseEvent *event) {
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);

    if(this->isMaximized()){
        settings.setValue("MainView/maximized", true);
    }
    else {
        settings.setValue("MainView/size", size());
        settings.setValue("MainView/pos", pos());
        settings.setValue("MainView/maximized", false);
    }

    if(_exitOnClose){
        emit exitCmd();
    }
    else{
        event->accept();
    }
}

void MainView::hideInTray() {
    bool oldExitOnClose = _exitOnClose;
    _exitOnClose = false;
    close();
    _exitOnClose = oldExitOnClose;
}

void MainView::hideEvent(QHideEvent *event) {
    if(!_exitOnClose){
        event->ignore();
        close();
    }
}

void MainView::resizeEvent(QResizeEvent *event) {
    Q_UNUSED(event)
    onResized();
}

void MainView::onResized() {
    _settingsBtn->move(_mainPage->width() - _settingsBtn->width() - 5, 5);
}

void MainView::onSettingsClick(){
    if(!_settingsMenu) {
        _settingsMenu = new QMenu(_mainPage);
        _settingsMenu->setTitle("Préférences");
        _settingsMenu->setLayoutDirection(Qt::LayoutDirection::RightToLeft);


        _showSyslogAction = new QAction(tr("&Logs"), _mainPage);
        connect(_showSyslogAction, SIGNAL(triggered()), this, SIGNAL(syslogWindowShowCmd()));
        _settingsMenu->addAction(_showSyslogAction);

        _settingsMenu->addSeparator();

        _hideInTrayAction = new QAction(tr("&Rester dans le tray"), _mainPage);
        _hideInTrayAction->setCheckable(true);
        connect(_hideInTrayAction, SIGNAL(triggered()), this, SLOT(onHideOnTrayToogled()));
        _settingsMenu->addAction(_hideInTrayAction);

        _autoCheckUpdatesAction = new QAction(tr("&Vérifier automatiquement les mises à jours"), _mainPage);
        _autoCheckUpdatesAction->setCheckable(true);
        connect(_autoCheckUpdatesAction, SIGNAL(triggered()), this, SLOT(onAutoCheckUpdateToogled()));
        _settingsMenu->addAction(_autoCheckUpdatesAction);

        QAction * mnt = new QAction(tr("&Mode exploitation"), _mainPage);
        mnt->setCheckable(true);
        mnt->setChecked(false);
        mnt->setEnabled(false);
        _settingsMenu->addAction(mnt);

        _settingsMenu->addSeparator();

        QAction *helpChapiAction = new QAction(tr("Aide sur &Chapi"), _mainPage);
        connect(helpChapiAction, SIGNAL(triggered()), this, SIGNAL(chapiHelpCmd()));
        _settingsMenu->addAction(helpChapiAction);

        QAction *helpChapiServerAction = new QAction(tr("Aide sur Chapi &Server"), _mainPage);
        connect(helpChapiServerAction, SIGNAL(triggered()), this, SIGNAL(chapiServerHelpCmd()));
        _settingsMenu->addAction(helpChapiServerAction);

        QAction *aboutAction = new QAction(tr("&A propos"), _mainPage);
        connect(aboutAction, SIGNAL(triggered()), this, SIGNAL(aboutCmd()));
        _settingsMenu->addAction(aboutAction);

         _settingsMenu->addSeparator();

        QAction *quitAction = new QAction(tr("&Quitter"), _mainPage);
        connect(quitAction, SIGNAL(triggered()), this, SIGNAL(exitCmd()));
        _settingsMenu->addAction(quitAction);
    }

    _hideInTrayAction->setChecked(!_exitOnClose);
    _autoCheckUpdatesAction->setChecked(_versionList.autocheck());
    _showSyslogAction->setEnabled(!_syslogStatus.isDisplayed());
    _settingsMenu->popup(_mainPage->mapToGlobal(QPoint((_mainPage->width() -5) - _settingsMenu->sizeHint().width(), 5)));
}

void MainView::onSyslogWindowVisibilityChanged(bool visible) {
    _showSyslogAction->setEnabled(!visible);
}

void MainView::onAutoCheckUpdateToogled() {
    _versionList.setAutocheck(!_versionList.autocheck());
    _autoCheckUpdatesAction->setChecked(_versionList.autocheck());
}

void MainView::onHideOnTrayToogled(){
    _exitOnClose = !_exitOnClose;
    _hideInTrayAction->setChecked(!_exitOnClose);
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);
    settings.setValue("MainView/exit_on_close", _exitOnClose);
}


void MainView::onDeviceListChanged() {
    QList<Device *> devices = _devList.devices();

    //remove old devices
    foreach(quint64 mac, _devViewList.keys()){
        if(!_devList.containsMac(mac)){
            removeDev(mac);
        }
    }

    foreach(Device *dev, devices) {
        if(!_devViewList.contains(dev->mac())){
            switch(dev->type()) {
                case Device::ChapiDev:      addDev(_chapiLayout, dev); break;
                case Device::VideoHub:      addDev(_vhLayout, dev); break;
                case Device::Atem:          addDev(_atemLayout, dev); break;
                case Device::ChapiServer:
                case Device::Router:
                case Device::UnknownDevice:
                default:
                    addDev(_otherLayout, dev);
                    break;
            }
        }
    }
}

void MainView::addDev(QBoxLayout *layout, Device *dev){
    if(_devCount[layout] == 0){
        QWidget *label = layout->itemAt(1)->widget();
        layout->removeItem(layout->itemAt(0)); //the first strech
        label->hide();
    }
    ++_devCount[layout];

    DeviceView *devView = new DeviceView(dev);
    connect(devView, SIGNAL(chapiViewCmd(ChapiDevice*)), this, SLOT(onChapiViewAsked(ChapiDevice*)));
    connect(devView, SIGNAL(videoHubViewCmd(VideoHubDevice*)), this, SLOT(onVideoHubViewAsked(VideoHubDevice*)));
    connect(devView, SIGNAL(atemViewCmd(AtemDevice*)), this, SLOT(onAtemViewAsked(AtemDevice*)));
    connect(devView, SIGNAL(chapiUpdateAsked(Device*)), this, SIGNAL(chapiUpdateAsked(Device*)));


    layout->insertWidget(layout->count()-1, devView);
    _devViewList.insert(dev->mac(), devView);
}

void MainView::removeDev(quint64 mac){
    DeviceView *view = _devViewList[mac];
    QBoxLayout *layout = (QBoxLayout*) view->parentWidget()->layout();
    int index = layout->indexOf(view);
    layout->removeWidget(view);
    delete view;
    layout->removeItem(layout->itemAt(index));
    delete layout->itemAt(index);

    --_devCount[layout];

    _devViewList.remove(mac);

    if(_devCount[layout] == 0) {
        QWidget *label = layout->itemAt(0)->widget();
        layout->insertStretch(0, 1);
        label->show();
    }
}


void MainView::onChapiViewAsked(ChapiDevice *dev){
    ChapiView *view = new ChapiView(dev, _devList);
    connect(view, SIGNAL(exitDeviceSettings()), this, SLOT(onDeviceSettingsExit()));

    QWidget *content = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addSpacing(1);
    QVBoxLayout *subLayout = new QVBoxLayout();
    subLayout->addSpacing(1);
    subLayout->addWidget(view);
    subLayout->addSpacing(1);
    mainLayout->addLayout(subLayout);
    mainLayout->addSpacing(1);
    content->setLayout(mainLayout);

    _stackedLayout->addWidget(content);
    _stackedLayout->setCurrentIndex(1);
    view->setFocus();
}

void MainView::onVideoHubViewAsked(VideoHubDevice *dev){
    VideoHubView *view = new VideoHubView(dev);
    connect(view, SIGNAL(exitDeviceSettings()), this, SLOT(onDeviceSettingsExit()));

    QWidget *content = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addSpacing(1);
    QVBoxLayout *subLayout = new QVBoxLayout();
    subLayout->addSpacing(1);
    subLayout->addWidget(view);
    subLayout->addSpacing(1);
    mainLayout->addLayout(subLayout);
    mainLayout->addSpacing(1);
    content->setLayout(mainLayout);

    _stackedLayout->addWidget(content);
    _stackedLayout->setCurrentIndex(1);
    view->setFocus();
}

void MainView::onAtemViewAsked(AtemDevice *dev){
    AtemView *view = new AtemView(dev);
    connect(view, SIGNAL(exitDeviceSettings()), this, SLOT(onDeviceSettingsExit()));

    QWidget *content = new QWidget();
    QHBoxLayout *mainLayout = new QHBoxLayout();
    mainLayout->addSpacing(1);
    QVBoxLayout *subLayout = new QVBoxLayout();
    subLayout->addSpacing(1);
    subLayout->addWidget(view);
    subLayout->addSpacing(1);
    mainLayout->addLayout(subLayout);
    mainLayout->addSpacing(1);
    content->setLayout(mainLayout);

    _stackedLayout->addWidget(content);
    _stackedLayout->setCurrentIndex(1);
    view->setFocus();
}

void MainView::onDeviceSettingsExit() {
    _stackedLayout->setCurrentIndex(0);
    _stackedLayout->removeWidget((QWidget*)sender());
    delete (QWidget*)sender();
    _stackedLayout->removeItem(_stackedLayout->itemAt(1));
    delete _stackedLayout->itemAt(1);
    _settingsBtn->move(_mainPage->width() - _settingsBtn->width() - 5, 5);
}
