#include "syslogview.h"

#include <QBoxLayout>
#include <QFrame>
#include <QStandardItemModel>
#include <QComboBox>
#include <QLabel>
#include <QListView>
#include <QHeaderView>
#include <QCloseEvent>
#include <QSettings>
#include <QStandardPaths>
#include <QSortFilterProxyModel>
#include <QHeaderView>

#include "../models/syslogentry.h"
#include "../models/syslogger.h"
#include "../models/syslogmodel.h"
#include "../models/devicelist.h"
#include "../utils/qlineview.h"
#include "../utils/syslogfilter.h"

#ifdef _WIN32
#include <Windows.h>
#include <Winbase.h>
#include <propsys.h>
#include <propkey.h>
#include <Propvarutil.h>
#include <qwindowdefs.h>

typedef HRESULT (STDAPICALLTYPE *SHGetPropertyStoreForWindow_t)(HWND hwnd, REFIID riid, void **ppv);
static SHGetPropertyStoreForWindow_t pSHGetPropertyStoreForWindow = 0;

void changeWindowIcon(HWND ewid){
    HMODULE shell32 = LoadLibraryW(L"shell32.dll");
    if(!shell32) {
        return;
    }

    pSHGetPropertyStoreForWindow = (SHGetPropertyStoreForWindow_t) GetProcAddress(shell32, "SHGetPropertyStoreForWindow");
    if(! pSHGetPropertyStoreForWindow){
        return;
    }

    IPropertyStore *windowPropertyStore;
    HRESULT result = pSHGetPropertyStoreForWindow((HWND) ewid, IID_PPV_ARGS(&windowPropertyStore));
    if(! SUCCEEDED(result)) {
        return;
    }
    PROPVARIANT newAppUId;
    result = InitPropVariantFromString(L"9F4C1111-9F79-1111-A8D0-E1D42DE1D5F3", &newAppUId);//random value, good format
    if (SUCCEEDED(result)) {
        windowPropertyStore->SetValue(PKEY_AppUserModel_ID, newAppUId);
        PropVariantClear(&newAppUId);
    }

    PROPVARIANT boolVariant;
    boolVariant.vt = VT_BOOL;
    boolVariant.boolVal = VARIANT_TRUE;
    windowPropertyStore->SetValue(PKEY_AppUserModel_PreventPinning, boolVariant);
    PropVariantClear(&boolVariant);

    windowPropertyStore->Release();
}

#endif

SyslogView::SyslogView(SyslogModel &syslogModel, DeviceList &devList) :
    QWidget(0), _logModel(syslogModel), _devList(devList)
{
    QIcon icon;
    icon.addFile(QStringLiteral(":/icons/imgs/settings.png"), QSize(), QIcon::Normal, QIcon::Off);
    setWindowIcon(icon);

#ifdef _WIN32
    changeWindowIcon((HWND) winId());
#endif

    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);
    resize(settings.value("LogView/size", QSize(450, 300)).toSize());
    move(settings.value("LogView/pos", QPoint(200, 200)).toPoint());
    if(settings.value("LogView/maximized", false).toBool()){
        showMaximized();
    }

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(QMargins());
    QFrame *toolbox = new QFrame();
    toolbox->setFrameStyle(QFrame::Panel | QFrame::Raised);
    toolbox->setLineWidth(1);
    //toolbox->setMaximumHeight(80);
    toolbox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QTableView *_view = new QLineView();
    _modelProxy = new SyslogFilter(_logModel);
    _view->setModel(_modelProxy);
    _view->setSortingEnabled(true);
    _view->horizontalHeader()->setSortIndicatorShown(true);
    _view->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    _view->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    _view->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    _view->horizontalHeader()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    _view->horizontalHeader()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    _view->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    _view->sortByColumn(2, Qt::AscendingOrder);
    _view->verticalHeader()->hide();
    _view->setShowGrid(false);
    _view->verticalHeader()->setDefaultSectionSize(_view->verticalHeader()->defaultSectionSize()*0.7);

    /*
    _view->setAlternatingRowColors(true);
    _view->setStyleSheet("QTableView{alternate-background-color: #EDF1F2; background-color: white;}");
    */
    _view->setStyleSheet("QTableView{color: white; background-color: black;} QTableView::item:selected {color:black; background-color: white}");

    _severityFilterModel = new QStandardItemModel();
    QStringList severities{"Urgence", "Alerte", "Critique", "Erreur", "Warning", "Information", "Détail", "Debug"};
    QStandardItem* item = new QStandardItem("Filter par sévérité");
    QStandardItem* initialItem = item;
    _severityFilterModel->insertRow(0, item);

    int i = _severityFilterModel->rowCount();
    foreach(QString label, severities){
        QStandardItem* item = new QStandardItem(label);
        item->setData(severities.indexOf(label), Qt::UserRole);
        //item->setIcon();
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setData(Qt::Checked, Qt::CheckStateRole);
        _severityFilterModel->insertRow(i, item);
        ++i;
    }
    QComboBox* severityCombo = new QComboBox();
    severityCombo->setModel(_severityFilterModel);
    severityCombo->setCurrentIndex(0);
    initialItem->setEnabled(false);
    QListView *comboListView = dynamic_cast<QListView*>(severityCombo->view());
    if(comboListView) {
        comboListView->setSpacing(2);
    }
    connect(_severityFilterModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), _modelProxy, SLOT(severitySelectionChanged(QModelIndex,QModelIndex)));


    _deviceFilterModel = new QStandardItemModel();
    item = new QStandardItem("Filter par machine");
    initialItem = item;
    _deviceFilterModel->insertRow(0, item);

    QList<Device*> listOfDevices;
    foreach(Device *dev, _devList.devices()){
        if(dev->isLoggable()){
            listOfDevices.push_back(dev);
        }
    }
    qSort(listOfDevices.begin(), listOfDevices.end(), [](Device *a, Device *b){
        if(a->type() > b->type()){
            return true;
        }
        if(a->type() < b->type()){
            return false;
        }
        return a->name() > b->name();
    });
    i = _deviceFilterModel->rowCount();
    foreach(Device *dev, listOfDevices){
        QStandardItem* item = new QStandardItem(dev->name());
        item->setData(dev->mac(), Qt::UserRole);
        //item->setIcon();
        item->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled);
        item->setData(Qt::Checked, Qt::CheckStateRole);
        _deviceFilterModel->insertRow(i, item);
        ++i;
    }

    QComboBox* deviceCombo = new QComboBox();
    deviceCombo->setModel(_deviceFilterModel);
    deviceCombo->setCurrentIndex(0);
    initialItem->setEnabled(false);
    comboListView = dynamic_cast<QListView*>(deviceCombo->view());
    if(comboListView) {
        comboListView->setSpacing(2);
    }
    connect(_deviceFilterModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), _modelProxy, SLOT(deviceSelectionChanged(QModelIndex,QModelIndex)));

    _modelProxy->setInitialFilter(_severityFilterModel, _deviceFilterModel);

    QHBoxLayout *toolboxLayout = new QHBoxLayout();
    toolboxLayout->addWidget(new QLabel("Sévérité:"));
    toolboxLayout->addSpacing(5);
    toolboxLayout->addWidget(severityCombo);
    toolboxLayout->addSpacing(25);
    toolboxLayout->addWidget(new QLabel("Machine:"));
    toolboxLayout->addSpacing(5);
    toolboxLayout->addWidget(deviceCombo);
    toolboxLayout->setContentsMargins(QMargins(11, 2, 11, 2));
    toolboxLayout->addStretch(1);
    toolbox->setLayout(toolboxLayout);

    //QtDebugMsg, QtWarningMsg, QtCriticalMsg, QtFatalMsg, QtSystemMsg = QtCriticalMsg

    mainLayout->addWidget(toolbox);


    mainLayout->addWidget(_view, 1);
    setLayout(mainLayout);

    QTimer::singleShot(500, this, SLOT(onStart()));

}

void SyslogView::closeEvent(QCloseEvent *event) {
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);

    if(this->isMaximized()){
        settings.setValue("LogView/maximized", true);
    }
    else {
        settings.setValue("LogView/size", size());
        settings.setValue("LogView/pos", pos());
        settings.setValue("LogView/maximized", false);
    }
    event->accept();
}


SyslogView::~SyslogView() {
    delete _deviceFilterModel;
    delete _severityFilterModel;
}

void SyslogView::onStart() {

}

