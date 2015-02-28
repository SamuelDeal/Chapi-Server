#include "deviceview.h"

#include <QPushButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QInputDialog>
#include <QTimer>
#include <QGraphicsColorizeEffect>
#include <QPropertyAnimation>

#include "../models/device.h"
#include "../models/chapidevice.h"
#include "../models/videohubdevice.h"
#include "../models/atemdevice.h"
#include "../utils/qclickablelabel.h"
#include "../utils/netutils.h"

DeviceView::DeviceView(Device *dev, QWidget *parent) :
    QFrame(parent)
{
    _dev = dev;
    _blinking = false;
    connect(_dev, SIGNAL(changed()), this, SLOT(onDevChanged()));
    connect(this, SIGNAL(doubleClick()), this, SLOT(onSettingsClick()));

    QColor color = palette().window().color();
    setStyleSheet("QFrame {background: rgb("+QString::number(color.red())+", "+QString::number(color.green())+", "+QString::number(color.blue())+"); }");

    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setLineWidth(1);

    QHBoxLayout *hlayout = new QHBoxLayout();
    setLayout(hlayout);

    hlayout->addStretch(1);

    QIcon icon;
    switch(dev->type()){
        case Device::ChapiDev:          icon.addFile(QStringLiteral(":/icons/imgs/chapi.png"), QSize(), QIcon::Normal, QIcon::Off); break;
        case Device::ChapiServer:       icon.addFile(QStringLiteral(":/icons/imgs/server.png"), QSize(), QIcon::Normal, QIcon::Off); break;
        case Device::VideoHub:          icon.addFile(QStringLiteral(":/icons/imgs/vh.png"), QSize(), QIcon::Normal, QIcon::Off); break;
        case Device::Atem:              icon.addFile(QStringLiteral(":/icons/imgs/atem.png"), QSize(), QIcon::Normal, QIcon::Off); break;
        case Device::Router:            icon.addFile(QStringLiteral(":/icons/imgs/router.png"), QSize(), QIcon::Normal, QIcon::Off); break;
        default:
            icon.addFile(QStringLiteral(":/icons/imgs/other.png"), QSize(), QIcon::Normal, QIcon::Off);
            break;
    }
    QClickableLabel *clickableLabel = new QClickableLabel();
    connect(clickableLabel, SIGNAL(doubleClick()), this, SLOT(onNameDoubleClick()));
    clickableLabel->setPixmap(icon.pixmap(32, 32));
    hlayout->addWidget(clickableLabel);

    _name = new QClickableLabel();
    connect(_name, SIGNAL(doubleClick()), this, SLOT(onNameDoubleClick()));
    QFont font(_name->font());
    font.setBold(true);
    _name->setFont(font);
    hlayout->addWidget(_name);

    hlayout->addStretch(1);

    _statusIcon = new QLabel();
    hlayout->addWidget(_statusIcon);

    if(_dev->isMonitorable()) {
        _monitorBtn = new QPushButton();
        _monitorBtn->setCheckable(true);
        _monitorBtn->setAutoDefault(false);
        _monitorBtn->setEnabled(false); //Not implemented
        QIcon icon2;
        icon2.addFile(QStringLiteral(":/icons/imgs/monitor.png"), QSize(), QIcon::Normal, QIcon::Off);
        _monitorBtn->setIcon(icon2);
        _monitorBtn->setIconSize(QSize(24,24));
        _monitorBtn->setToolTip("Surveiller le périphérique");
        hlayout->addWidget(_monitorBtn);
        connect(_monitorBtn, SIGNAL(clicked()), this, SLOT(onMonitorClick()));
    }
    else {
        hlayout->addSpacerItem(new QSpacerItem(42, 24));
    }

    if(_dev->isConfigurable()) {
        _settingsBtn = new QPushButton();
        _settingsBtn->setAutoDefault(false);
        QIcon icon3;
        icon3.addFile(QStringLiteral(":/icons/imgs/settings.png"), QSize(), QIcon::Normal, QIcon::Off);
        _settingsBtn->setIcon(icon3);
        _settingsBtn->setIconSize(QSize(24,24));
        hlayout->addWidget(_settingsBtn);
        _settingsBtn->setToolTip("Configurer le périphérique");
        connect(_settingsBtn, SIGNAL(clicked()), this, SLOT(onSettingsClick()));
    }
    else {
        hlayout->addSpacerItem(new QSpacerItem(42, 24));
    }

    if(_dev->isUpdatable()) {
        _updateBtn = new QPushButton();
        _updateBtn->setAutoDefault(false);
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/icons/imgs/monitor.png"), QSize(), QIcon::Normal, QIcon::Off);
        _updateBtn->setIcon(icon4);
        _updateBtn->setIconSize(QSize(24,24));
        hlayout->addWidget(_updateBtn);
        _updateBtn->setToolTip("Mettre à jour");
        connect(_updateBtn, SIGNAL(clicked()), this, SLOT(onUpdateClick()));
    }
    else {
        hlayout->addSpacerItem(new QSpacerItem(42, 24));
    }

    if(_dev->isIdentifiable()) {
        _blinkBtn = new QPushButton();
        _blinkBtn->setAutoDefault(false);
        QIcon icon4;
        icon4.addFile(QStringLiteral(":/icons/imgs/blink.png"), QSize(), QIcon::Normal, QIcon::Off);
        _blinkBtn->setIcon(icon4);
        _blinkBtn->setIconSize(QSize(24,24));
        hlayout->addWidget(_blinkBtn);
        _blinkBtn->setToolTip("Faire clignoter le périphérique");
        connect(_blinkBtn, SIGNAL(clicked()), this, SLOT(onBlinkClick()));
    }
    else {
        hlayout->addSpacerItem(new QSpacerItem(42, 24));
    }

    onDevChanged();
}

void DeviceView::mouseDoubleClickEvent(QMouseEvent * event){
    QFrame::mouseDoubleClickEvent(event);
    emit doubleClick();
}


void DeviceView::onNameDoubleClick() {
    if(_dev->isCurrentComputer()){
        return;
    }
    bool ok = true;
    QString text = QInputDialog::getText(this, tr("Renommer"), tr("Nouveau Nom:"), QLineEdit::Normal, _dev->name(), &ok);
    if(ok && !text.isEmpty()) {
        _name->setText(text);
        _dev->setName(text);
    }
}

void DeviceView::onDevChanged() {
    _name->setText(_dev->name().isEmpty()? "Non Configuré" : _dev->name());

    QIcon iconLed;
    QString status;
    switch(_dev->status()){
        case Device::Unreachable:
            status = "Injoignable";
            break;
        case Device::CurrentComputer:
            status = "Ordinateur courant, serveur lancé";
            break;
        case Device::Located:
            status = "Localisé";
            break;
        case Device::Unconfigured:
            status = "Non configuré";
            break;
        case Device::ApplyingConfig:
            status = "Configuration en cours...";
            break;
        case Device::ReadingConfig:
            status = "Lecture de la configuration...";
            break;
        case Device::Connecting:
            status = "Connection en cours...";
            break;
        case Device::Connected:
            status = "Connecté";
            break;
        case Device::HubUnreachable:
            status = "Ne vit pas le hub";
            break;
        case Device::ConnectingToHub:
            status = "Connection au hub en cours...";
            break;
        case Device::Ready:
            status = "Fonctionnel";
            break;

        default:
            status = "Inconnu";
            break;
    }

    switch(_dev->simpleStatus()){
        case Device::Green:
            iconLed.addFile(QStringLiteral(":/icons/imgs/green.png"), QSize(), QIcon::Normal, QIcon::Off);
            break;

        case Device::Yellow:
            iconLed.addFile(QStringLiteral(":/icons/imgs/yellow.png"), QSize(), QIcon::Normal, QIcon::Off);
            break;

        default:
        case Device::Red:
            iconLed.addFile(QStringLiteral(":/icons/imgs/red.png"), QSize(), QIcon::Normal, QIcon::Off);
            break;
    }

    _statusIcon->setPixmap(iconLed.pixmap(24, 24));
    _statusIcon->setToolTip("Status: "+status);

    if(_dev->isMonitorable()) {
        _monitorBtn->setChecked(_dev->isMonitored());
        _monitorBtn->setToolTip(QString("Surveiller le périphérique\n(")+(_dev->isMonitored() ? "Activé)" : "Désactivé)"));
    }

    if(_dev->isConfigurable()) {
        _settingsBtn->setDisabled(!_dev->isConfigurableNow());
    }

    if(_dev->isIdentifiable()) {
        _blinkBtn->setDisabled(_blinking || !_dev->isIdentifiableNow());
    }

    if(_dev->isUpdatable()) {
        _updateBtn->setDisabled(!_dev->isUpdatableNow());
    }

    QString version = "Version: " + (_dev->version().isEmpty() ? "Non détectée" : _dev->version()) + "\n";
    QString type;
    switch (_dev->type()) {
        case Device::Atem:                  type = "ATEM Switcher"; break;
        case Device::ChapiDev:              type = "Chapi"; break;
        case Device::ChapiServer:           type = "Serveur Chapi"; break;
        case Device::VideoHub:              type = "VideoHub"; break;
        case Device::Router:                type = "Routeur"; version = ""; break;

        case Device::UnknownDevice:
        default:
            type = "Autre";
            version = "";
            break;
    }
    QString name = _dev->name().isEmpty() ? "Nom inconnu" : _dev->name();
    QString ip = _dev->ip().isEmpty() ? "Non détecté" : _dev->ip();
    setToolTip(name + " ("+type+")\n"+version+"Ip: "+ip+"\nMac: "+NetUtils::macToStr(_dev->mac())+"\nStatus: "+status);
}


void DeviceView::onMonitorClick(){
    _dev->setMonitored(!_dev->isMonitored());
}

void DeviceView::onSettingsClick(){
    if(_dev->type() == Device::ChapiDev){
        emit chapiViewCmd(dynamic_cast<ChapiDevice*>(_dev));
    }
    else if(_dev->type() == Device::VideoHub){
        emit videoHubViewCmd(dynamic_cast<VideoHubDevice*>(_dev));
    }
    else if(_dev->type() == Device::Atem){
        emit atemViewCmd(dynamic_cast<AtemDevice*>(_dev));
    }
}

void DeviceView::onBlinkingFinished(){
    _blinking = false;
    if(_dev->isIdentifiable()) {
        _blinkBtn->setDisabled(_blinking);
    }

    _colorisation = 0;
    _colorizeEffect->setStrength(0);
    update();
}

void DeviceView::onBlinkClick(){
    _blinking = true;
    _blinkBtn->setDisabled(true);
    _dev->blink();

    _colorizeEffect = new QGraphicsColorizeEffect();
    _colorizeEffect->setColor(Qt::yellow);
    _colorisation = 0;
    _colorizeEffect->setStrength(_colorisation);
    setGraphicsEffect(_colorizeEffect);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "colorisation");
    animation->setDuration(200);
    animation->setLoopCount(10);
    animation->setStartValue(0.0);
    animation->setEndValue(0.5);
    animation->setEasingCurve(QEasingCurve::CosineCurve);
    animation->start();

    QTimer::singleShot(2100, this, SLOT(onBlinkingFinished()));
}


qreal DeviceView::getColorisation() {
    return _colorisation;
}

void DeviceView::setColorisation(qreal val) {
    _colorisation = val;
    _colorizeEffect->setStrength(val);
    update();
}

void DeviceView::onUpdateClick() {
    emit chapiUpdateAsked(_dev);
}
