#include "networksettingsview.h"

#include <QFormLayout>
#include <QRadioButton>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QTimer>
#include <QLabel>
#include <QMessageBox>

#include "../utils/netutils.h"
#include "../utils/qipwidget.h"
#include "../utils/qradiobox.h"
#include "../utils/errorlist.h"
#include "../models/networkconfig.h"
#include "../models/devicelist.h"
#include "../models/devicelist.h"

NetworkSettingsView::NetworkSettingsView(NetworkConfig *cfg, Device *dev, DeviceList *devList, QWidget *parent) :
    QDialog(parent)
{
    _cfg = cfg;
    _dev = dev;
    _devList = devList;
    setModal(true);
    setWindowTitle(tr("Configuration réseau"));
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout *layout = new QVBoxLayout();

    QHBoxLayout *descrLayout = new QHBoxLayout();

    QLabel *descrIcon = new QLabel();
    QIcon icon;
    icon.addFile(QStringLiteral(":/icons/network.png"), QSize(), QIcon::Normal, QIcon::Off);
    descrIcon->setPixmap(icon.pixmap(32, 32));

    QLabel *descrLabel = new QLabel("Veuillez configurer l'adresse ip du Chapi");

    descrLayout->addSpacing(10);
    descrLayout->addWidget(descrIcon);
    descrLayout->addWidget(descrLabel, 1);
    layout->addLayout(descrLayout);
    layout->addSpacing(5);

    _dhcpButton = new QRadioButton(tr("&Obtenir une adresse IP automatiquement"));
    layout->addWidget(_dhcpButton);
    connect(_dhcpButton, SIGNAL(toggled(bool)), this, SLOT(onDhcpToggled(bool)));

    _staticIpBox = new QRadioBox(tr("U&tiliser l'adresse IP suivante:\t\t\t"));
    _staticIpBox->setCheckable(true);
    connect(_staticIpBox, SIGNAL(toggled(bool)), this, SLOT(onStaticIpToggled(bool)));

    QFormLayout *formLayout = new QFormLayout();
    _staticIpBox->setLayout(formLayout);
    _ip = new QIpWidget();
    _mask = new QIpWidget();
    _gateway = new QIpWidget();
    formLayout->addRow(tr("Adresse &Ip:"), _ip);
    formLayout->addRow(tr("Masque &sous-réseau:"), _mask);
    formLayout->addRow(tr("Passerelle par &défaut:"), _gateway);
    connect(_ip, SIGNAL(editingFinished()), this, SLOT(onIpDefined()));

    layout->addWidget(_staticIpBox);
    layout->addSpacing(5);

    QDialogButtonBox *btnBox = new QDialogButtonBox();
    QPushButton *cancelBtn = btnBox->addButton("&Annuler", QDialogButtonBox::RejectRole);
    connect(cancelBtn, SIGNAL(clicked()), this, SLOT(onCancelClicked()));
    QPushButton *okBtn = btnBox->addButton("Ok", QDialogButtonBox::ApplyRole);
    connect(okBtn, SIGNAL(clicked()), this, SLOT(onOkClicked()));

    layout->addWidget(btnBox);
    setLayout(layout);

    if(_cfg->useDHCP){
        _dhcpButton->setChecked(true);
        _staticIpBox->setChecked(false);
    }
    _ip->setValue(QHostAddress(_cfg->ip));
    _mask->setValue(QHostAddress(_cfg->netmask));
    _gateway->setValue(QHostAddress(_cfg->gateway));

    QTimer::singleShot(1, this, SLOT(onViewInited()));
}

void NetworkSettingsView::onViewInited() {
    _dhcpButton->setStyleSheet("margin-left:"+QString::number(_staticIpBox->getLeftMargin())+"px;");
}

void NetworkSettingsView::onDhcpToggled(bool value) {
    if(!value){
        return;
    }
    _staticIpBox->setChecked(false);
}

void NetworkSettingsView::onStaticIpToggled(bool value) {
    if(!value){
        return;
    }
    _dhcpButton->setChecked(false);
    _ip->setChildFocus();
}

void NetworkSettingsView::onCancelClicked() {
    close();
}

void NetworkSettingsView::onOkClicked() {
    if(_dhcpButton->isChecked()){
        if(!NetUtils::isValidAddress(_ip->getValue())){
            _ip->setValue(QHostAddress());
            _mask->setValue(QHostAddress());
            _gateway->setValue(QHostAddress());
        }
        else {
            if(!NetUtils::isValidMask(_mask->getValue())){
                _mask->setValue(NetUtils::getDefaultSubmask(_ip->getValue()));
            }
            if(!NetUtils::isValidAddress(_ip->getValue(), _mask->getValue())){
                _ip->setValue(QHostAddress());
                _mask->setValue(QHostAddress());
                _gateway->setValue(QHostAddress());
            }
            else {
                QHostAddress gateway = _gateway->getValue();
                int mask = NetUtils::ipToMaskPrefix(_mask->getValue());
                if(!NetUtils::isValidAddress(gateway, mask) || !NetUtils::inSameNetwork(_ip->getValue(), gateway, mask) || (_ip->getValue() == _gateway->getValue())) {
                    _gateway->setValue(QHostAddress());
                }
            }
        }
    }
    else {
        //non-ignorable errors
        if(!NetUtils::isValidAddress(_ip->getValue())){
            QMessageBox::critical(this, "Configuration du réseau invalide", "L'adresse ip renseignée n'est pas une adresse valide", QMessageBox::Ok, QMessageBox::Ok);
            _ip->errorEffect();
            return;
        }
        if(!NetUtils::isValidMask(_mask->getValue())){
            QMessageBox::critical(this, "Configuration du réseau invalide", "Le masque sous-réseau renseigné n'est pas un masque valide", QMessageBox::Ok, QMessageBox::Ok);
            _mask->errorEffect();
            return;
        }
        if(!NetUtils::isValidAddress(_ip->getValue(), _mask->getValue())){
            QMessageBox::critical(this, "Configuration du réseau invalide", "L'adresse ip renseignée n'est pas une adresse valide", QMessageBox::Ok, QMessageBox::Ok);
            _ip->errorEffect();
            _mask->errorEffect();
            return;
        }

        ErrorList errorList("Configuration du réseau", this);

        foreach(Device* dev, _devList->devices()){
            if((dev != _dev) && (!dev->ip().isEmpty()) && (QHostAddress(dev->ip()) == _ip->getValue())) {
                errorList.addError(true, "Cette adresse est déjà utilisé par un autre appareil ("+_dev->name()+").\n"
                    "Si vous configurez cette adresse, le réseau sera fortement perturbé.\n"
                    "Êtes vous sûr de vouloir continuer?", []{}, [&]{
                    _ip->errorEffect();
                });
                break;
            }
        }

        bool devCanSeeServer = false;
        bool serverCanSeeDev = false;
        foreach(QNetworkInterface inet, QNetworkInterface::allInterfaces()) {
            if(((inet.flags() & QNetworkInterface::IsLoopBack) == 0) && ((inet.flags() & QNetworkInterface::IsUp) == QNetworkInterface::IsUp)) {
                QHostAddress currentIp = NetUtils::getIp(inet);
                if(!currentIp.isNull()){
                    int netMask = NetUtils::getMaskPrefix(inet);
                    if(NetUtils::inSameNetwork(_ip->getValue(), currentIp, netMask)){
                        serverCanSeeDev = true;
                    }
                    if(NetUtils::inSameNetwork(_ip->getValue(), currentIp, _mask->getValue())){
                        devCanSeeServer = true;
                    }
                }
            }
        }

        if(!devCanSeeServer || !serverCanSeeDev) {
            errorList.addError(true, "L'adresse configuré n'est pas sur le réseau de ce serveur.\n"
                    "Si vous configurez cette adresse, le Chapi et le serveur ne risquent de ne pas pouvoir se voir .\n"
                    "Êtes vous sûr de vouloir continuer?", []{}, [&]{
                _ip->errorEffect();
                _mask->errorEffect();
            });
        }

        if(!_gateway->isEmpty() && !NetUtils::isValidAddress(_gateway->getValue(), _mask->getValue())) {
            errorList.addError(false, "La passerelle renseignée n'est pas une adresse valide.\n"
                    "Voulez vous continuer sans passerelle réseau?", [&]{
                _gateway->setValue(QHostAddress());
            }, [&]{
                _gateway->errorEffect();
            });
        }
        else if(!_gateway->isEmpty() && (!NetUtils::inSameNetwork(_ip->getValue(), _gateway->getValue(), _mask->getValue()) || (_ip->getValue() == _gateway->getValue()))){
            errorList.addError(false, "La passerelle et l'adresse ip renseignées ne sont pas compatibles.\n"
                    "Voulez vous continuer sans passerelle réseau?", [&]{
                _gateway->setValue(QHostAddress());
            }, [&]{
                _gateway->errorEffect();
            });
        }

        errorList.run();
        if(errorList.isAborted()){
            return;
        }
    }

    _cfg->useDHCP = _dhcpButton->isChecked();
    _cfg->ip = _ip->getValue().toString();
    _cfg->netmask = _mask->getValue().toString();
    _cfg->gateway = _gateway->getValue().toString();
    accept();
}

void NetworkSettingsView::onIpDefined() {
    if(!_gateway->isEmpty()){
        return;
    }
    QHostAddress addr = _ip->getValue();
    _mask->setValue(NetUtils::getDefaultSubmask(addr));
}
