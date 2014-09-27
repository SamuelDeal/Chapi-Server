#include "syslogview.h"

#include <QBoxLayout>
#include <QFrame>
#include <QStandardItemModel>
#include <QComboBox>
#include <QLabel>
#include <QListView>
#include <QTableView>
#include <QHeaderView>
#include <QCloseEvent>
#include <QSettings>
#include <QStandardPaths>
#include <QSortFilterProxyModel>

#include "../models/syslogentry.h"
#include "../models/syslogger.h"
#include "../models/syslogmodel.h"
#include "../models/devicelist.h"
#include "../utils/syslogfilter.h"

SyslogView::SyslogView(SyslogModel *syslogModel, DeviceList *devList) :
    QWidget(0)
{
    _logModel = syslogModel;
    _devList = devList;

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

    QTableView *_view = new QTableView();
    _modelProxy = new SyslogFilter(_logModel);
    _view->setModel(_modelProxy);
    _view->setSortingEnabled(true);
    _view->setSelectionBehavior(QAbstractItemView::SelectRows);
    _view->horizontalHeader()->setSortIndicatorShown(true);
    _view->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Stretch);
    _view->verticalHeader()->hide();

    _severityFilterModel = new QStandardItemModel();
    QStringList severities{"Urgence", "Alerte", "Critique", "Erreur", "Warning", "Information", "Détail", "Debug"};
    QStandardItem* item = new QStandardItem("Filter par sévérité");
    QStandardItem* initialItem = item;
    _severityFilterModel->insertRow(0, item);

    int i = _severityFilterModel->rowCount();
    foreach(QString label, severities){
        QStandardItem* item = new QStandardItem(label);
        item->setData(i, Qt::UserRole);
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
    foreach(Device *dev, _devList->devices()){
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
