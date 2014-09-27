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

#include "../models/devicelist.h"
#include "deviceview.h"
#include "chapiview.h"
#include "videohubview.h"
#include "atemview.h"

MainView::MainView(DeviceList *devList, QWidget *parent) :
    QWidget(parent)
{
    setWindowTitle(tr("Chapi serveur"));

    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);
    resize(settings.value("MainView/size", QSize(450, 300)).toSize());
    move(settings.value("MainView/pos", QPoint(200, 200)).toPoint());
    if(settings.value("MainView/maximized", false).toBool()){
        showMaximized();
    }

    _devList = devList;
    connect(_devList, SIGNAL(deviceListChanged()), this, SLOT(onDeviceListChanged()));
    initTabs();

    _stackedLayout = new QStackedLayout();
    QWidget *mainPage = new QWidget();
    _stackedLayout->addWidget(mainPage);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setMargin(0);
    mainLayout->addWidget(_tabs);
    mainPage->setLayout(mainLayout);


    setLayout(_stackedLayout);

    onDeviceListChanged();
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
    icon1.addFile(QStringLiteral(":/icons/chapi.png"), QSize(), QIcon::Normal, QIcon::Off);
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
    icon2.addFile(QStringLiteral(":/icons/vh.png"), QSize(), QIcon::Normal, QIcon::Off);
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
    icon3.addFile(QStringLiteral(":/icons/atem.png"), QSize(), QIcon::Normal, QIcon::Off);
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
    icon4.addFile(QStringLiteral(":/icons/other.png"), QSize(), QIcon::Normal, QIcon::Off);
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

    event->accept();
}



void MainView::onDeviceListChanged() {
    qDebug() << "onDevListCHanged";
    QList<Device *> devices = _devList->devices();

    //remove old devices
    foreach(quint64 mac, _devViewList.keys()){
        if(!_devList->containsMac(mac)){
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
}
