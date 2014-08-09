#include "chapiview.h"

#include <QVBoxLayout>
#include <QPushButton>
#include <QInputDialog>
#include <QKeyEvent>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QStandardItemModel>
#include <QGroupBox>
#include <QFormLayout>
#include <QSignalMapper>

#include "networksettingsview.h"
#include "../utils/qclickablelabel.h"
#include "../models/device.h"
#include "../models/devicelist.h"
#include "../models/chapidevice.h"
#include "../models/videohubdevice.h"
#include "../models/atemdevice.h"

ChapiView::ChapiView(ChapiDevice *dev, DeviceList *devList, QWidget *parent) :
    QIntegratedFrame(parent), _netCfg(dev->networkConfig())
{
    _dev = dev;
    connect(_dev, SIGNAL(changed()), this, SLOT(onCheckOk()));

    _devList = devList;
    connect(_devList, SIGNAL(deviceListChanged()), this, SLOT(onDeviceListChanged()));

    _previousMacAddress = 0;

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    setMaximumWidth(450);

    QHBoxLayout *labelBox = new QHBoxLayout();
    labelBox->addStretch(1);

    QIcon icon;
    icon.addFile(QStringLiteral(":/icons/chapi.png"), QSize(), QIcon::Normal, QIcon::Off);
    QClickableLabel *clickableLabel = new QClickableLabel();
    connect(clickableLabel, SIGNAL(doubleClick()), this, SLOT(onNameDoubleClick()));
    clickableLabel->setPixmap(icon.pixmap(32, 32));
    labelBox->addWidget(clickableLabel);

    _nameLabel = new QClickableLabel();
    connect(_nameLabel, SIGNAL(doubleClick()), this, SLOT(onNameDoubleClick()));
    QFont font(_nameLabel->font());
    font.setBold(true);
    _nameLabel->setFont(font);
    _nameLabel->setText(_dev->name());
    labelBox->addWidget(_nameLabel);

    labelBox->addStretch(1);
    layout->addLayout(labelBox);
    layout->addStretch(2);

    QIcon icon2;
    icon2.addFile(QStringLiteral(":/icons/network.png"), QSize(), QIcon::Normal, QIcon::Off);
    QPushButton *networkButton = new QPushButton(icon2, tr("réseau"));
    networkButton->setIconSize(QSize(24, 24));

    layout->addWidget(networkButton);
    connect(networkButton, SIGNAL(clicked()), this, SLOT(onNetworkBtnClicked()));

    _targetBox = new QComboBox();
    layout->addWidget(_targetBox);

    connect(_targetBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTargetSelected(int)));
    onDeviceListChanged();

    _outputBox = new QComboBox();
    layout->addWidget(_outputBox);
    _outputBox->setVisible(false);

    _inputsBox = new QGroupBox("Sources: ");
    layout->addWidget(_inputsBox);
    _inputsBox->setVisible(false);

    _inputsLayout = new QFormLayout();
    _inputsBox->setLayout(_inputsLayout);

    for(int i = 0; i < _dev->nbrButtons(); i++) {
        _inputsLayout->addWidget(new QLabel("Bouton "+QString::number(i+1)+":"));
        QComboBox *inputBtnBox = new QComboBox();
        _inputsLayout->addWidget(inputBtnBox);
        _inputBoxes.append(inputBtnBox);
        connect(inputBtnBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCheckOk()));
    }


    connect(_outputBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCheckOk()));

    layout->addStretch(2);

    QDialogButtonBox *btnBox = new QDialogButtonBox();
    QPushButton *restartBtn = btnBox->addButton("Redémarer", QDialogButtonBox::ActionRole);
    connect(restartBtn, SIGNAL(clicked()), this, SLOT(onRestartClicked()));
    QPushButton *cancelBtn = btnBox->addButton("Annuler", QDialogButtonBox::RejectRole);
    connect(cancelBtn, SIGNAL(clicked()), this, SIGNAL(exitDeviceSettings()));
    _okBtn = btnBox->addButton("Ok", QDialogButtonBox::ApplyRole);
    connect(_okBtn, SIGNAL(clicked()), this, SLOT(onOkClicked()));
    layout->addWidget(btnBox);
    setFocus();
    layout->setSizeConstraint(QLayout::SetFixedSize);

    _networkSet = dev->isConfigured();
    onCheckOk();
}

void ChapiView::onDeviceListChanged() {
    disconnect(_targetBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTargetSelected(int)));
    quint64 mac = _targetBox->currentData().toULongLong();
    _targetBox->clear();

    QIcon icon3;
    icon3.addFile(QStringLiteral(":/icons/vh.png"), QSize(), QIcon::Normal, QIcon::Off);
    QIcon icon4;
    icon4.addFile(QStringLiteral(":/icons/atem.png"), QSize(), QIcon::Normal, QIcon::Off);

    _targetBox->insertItem(0, "Atems:", QVariant(0));
    qobject_cast<QStandardItemModel *>(_targetBox->model())->item(0)->setEnabled(false);

    foreach(Device* devTarget, _devList->devices()) {
        if(devTarget->type() == Device::Atem) {
            _targetBox->addItem(icon4, devTarget->name(), QVariant(devTarget->mac()));
        }
        else if(devTarget->type() == Device::VideoHub){
            _targetBox->insertItem(0, icon3, devTarget->name(), QVariant(devTarget->mac()));
        }
    }
    _targetBox->insertItem(0, "Vidéohubs:", QVariant(0));
    qobject_cast<QStandardItemModel *>(_targetBox->model())->item(0)->setEnabled(false);
    _targetBox->insertItem(0, "Sélectionnez la cible", QVariant(0));
    connect(_targetBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTargetSelected(int)));
    if(mac == 0) {
        _targetBox->setCurrentIndex(0);
    }
    else {
        _targetBox->setCurrentIndex(_targetBox->findData(QVariant(mac)));
    }
    qobject_cast<QStandardItemModel *>(_targetBox->model())->item(0)->setEnabled(false);
}

void ChapiView::onTargetSelected(int index) {
    if(index == 0) {
        return;
    }
    quint64 mac = _targetBox->itemData(index).toULongLong();
    if((mac == 0) || (mac == _previousMacAddress)){
        return;
    }
    QIcon icon;
    icon.addFile(QStringLiteral(":/icons/output.png"), QSize(), QIcon::Normal, QIcon::Off);

    QIcon icon2;
    icon2.addFile(QStringLiteral(":/icons/input.png"), QSize(), QIcon::Normal, QIcon::Off);

    Device* devTarget = _devList->deviceByMac(mac);
    if(devTarget->type() == Device::VideoHub) {
        VideoHubDevice *target = dynamic_cast<VideoHubDevice*>(devTarget);
        QMap<quint8, QString> outputLabels = target->outputLabels();
        foreach(quint8 index, outputLabels.keys()){
            _outputBox->addItem(icon, outputLabels[index], QVariant(index));
        }

        QMap<quint8, QString> inputLabels = target->inputLabels();
        quint8 btnIndex = 0;
        foreach(QComboBox *inputBox, _inputBoxes){
            inputBox->clear();
            inputBox->addItem("Aucune source", QVariant((qint32)ChapiDevice::NO_SOURCE));
            foreach(quint8 index, inputLabels.keys()){
                inputBox->addItem(icon2, inputLabels[index], QVariant((qint32)index));
            }
            inputBox->insertItem(0, "Sélectionnez la source", QVariant((qint32)-2));
            if(mac == _dev->targetMac()){
                inputBox->setCurrentIndex(inputBox->findData(QVariant(_dev->inputIndex(btnIndex))));
            }
            else {
                inputBox->setCurrentIndex(0);
            }
            ++btnIndex;
            qobject_cast<QStandardItemModel *>(inputBox->model())->item(0)->setEnabled(false);
        }
    }

    _previousMacAddress = mac;
    _outputBox->insertItem(0, "Sélectionnez la sortie", QVariant((qint32)-2));
    if(mac == _dev->targetMac()){
        _outputBox->setCurrentIndex(_outputBox->findData(_dev->outputIndex()));
    }
    else {
        _outputBox->setCurrentIndex(0);
    }
    qobject_cast<QStandardItemModel *>(_outputBox->model())->item(0)->setEnabled(false);
    _outputBox->setVisible(true);
    _inputsBox->setVisible(true);
}

void ChapiView::onCheckOk(int unused) {
    Q_UNUSED(unused);
    if(!_networkSet){
        _okBtn->setEnabled(false);
    }
    else if(!_dev->isConfigurableNow()){
        _okBtn->setEnabled(false);
    }
    else if(_targetBox->currentData().toULongLong() == 0){
        _okBtn->setEnabled(false);
    }
    else if(_outputBox->currentData().toInt() < 0){
        _okBtn->setEnabled(false);
    }
    else {
        foreach(QComboBox *inputBox, _inputBoxes){
            if(inputBox->currentData().toInt() < 0){
                _okBtn->setEnabled(false);
                return;
            }
        }
        _okBtn->setEnabled(true);
    }
}

void ChapiView::onNetworkBtnClicked() {
    NetworkSettingsView net(&_netCfg, _dev, _devList, this);
    int result = net.exec();
    if(result == QDialog::Accepted) {
        _networkSet = true;
    }
}

void ChapiView::onNameDoubleClick() {
    bool ok = true;
    QString text = QInputDialog::getText(this, tr("Renommer"),
                tr("Nouveau Nom:"), QLineEdit::Normal, _nameLabel->text(), &ok);
    if(ok && !text.isEmpty()) {
        _nameLabel->setText(text);
        _newName = text;
    }
}

void ChapiView::onRestartClicked() {
    _dev->restart();
    emit exitDeviceSettings();
}

void ChapiView::onOkClicked() {
    if(!_newName.isEmpty()){
        _dev->setName(_newName);
    }
    quint64 targetMac = _targetBox->itemData(_targetBox->currentIndex()).toULongLong();
    _dev->setTarget(dynamic_cast<TargetableDevice*>(_devList->deviceByMac(targetMac)));
    _dev->setOutputIndex(_outputBox->itemData(_outputBox->currentIndex()).toInt());
    quint8 btnIndex = 0;
    foreach(QComboBox *inputBox, _inputBoxes){
        _dev->setInputIndex(btnIndex++, inputBox->itemData(inputBox->currentIndex()).toInt());
    }
    _dev->saveConfig(_netCfg);
    emit exitDeviceSettings();
}
