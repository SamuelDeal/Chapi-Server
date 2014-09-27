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
#include <QScrollArea>
#include <QPropertyAnimation>

#include "networksettingsview.h"
#include "outputview.h"
#include "../utils/qclickablelabel.h"
#include "../utils/qselectdialog.h"
#include "../utils/qcolorizablebutton.h"
#include "../models/device.h"
#include "../models/devicelist.h"
#include "../models/chapidevice.h"
#include "../models/videohubdevice.h"
#include "../models/atemdevice.h"


ChapiView::ChapiView(ChapiDevice *dev, DeviceList *devList, QWidget *parent) :
    QIntegratedFrame(parent), _netCfg(dev->networkConfig())
{
    _dev = dev;
    _target = NULL;
    connect(_dev, SIGNAL(changed()), this, SLOT(onCheckOk()));

    _devList = devList;
    connect(_devList, SIGNAL(deviceListChanged()), this, SLOT(onDeviceListChanged()));

    setMaximumWidth(450);

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

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
    _networkButton = new QColorizableButton(icon2, tr("réseau"));
    _networkButton->setIconSize(QSize(24, 24));

    _networkButtonAnim = NULL;
    if(!_dev->isConfigured()){
        _networkButtonAnim = new QPropertyAnimation(_networkButton, "colorisation");
        _networkButtonAnim->setDuration(1000);
        _networkButtonAnim->setLoopCount(-1);
        _networkButtonAnim->setStartValue(0.0);
        _networkButtonAnim->setEndValue(0.5);
        _networkButtonAnim->setEasingCurve(QEasingCurve::CosineCurve);
        _networkButtonAnim->start();
    }

    layout->addWidget(_networkButton);
    connect(_networkButton, SIGNAL(clicked()), this, SLOT(onNetworkBtnClicked()));

    QFormLayout *formLayout = new QFormLayout();
    _targetBox = new QComboBox();
    formLayout->addRow(tr("&Cible:"), _targetBox);
    layout->addLayout(formLayout);

    _tabs = new QTabWidget(this);
    _tabs->setIconSize(QSize(24, 24));
    QFile styleFile(":/styles/tabs.qss");
    styleFile.open(QFile::ReadOnly );
    QString style(styleFile.readAll() );
    _tabs->setStyleSheet(style);

    QScrollArea* scrollArea = new QScrollArea();
    QWidget *content = new QWidget();
    _outputLayout = new QVBoxLayout();
    content->setLayout(_outputLayout);
    content->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QIcon iconOutput;
    iconOutput.addFile(QStringLiteral(":/icons/output.png"), QSize(), QIcon::Normal, QIcon::Off);
    _tabs->addTab(scrollArea, iconOutput, tr("Sorties"));
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(content);

    QIcon iconAdd;
    iconAdd.addFile(QStringLiteral(":/icons/add.png"), QSize(), QIcon::Normal, QIcon::Off);
    _addOutputBtn = new QPushButton(iconAdd, "Ajouter une sortie");
    _addOutputBtn->setMinimumHeight(30);
    _addOutputBtn->setMaximumWidth(200);
    connect(_addOutputBtn, SIGNAL(clicked()), this, SLOT(onAddBtnClicked()));
    _outputLayout->addStretch(1);
    _outputLayout->addWidget(_addOutputBtn);

    scrollArea = new QScrollArea();
    content = new QWidget();
    _inputLayout = new QVBoxLayout();
    content->setLayout(_inputLayout);
    content->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QIcon iconInput;
    iconInput.addFile(QStringLiteral(":/icons/input.png"), QSize(), QIcon::Normal, QIcon::Off);
    _tabs->addTab(scrollArea, iconInput, tr("Entrées"));
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(content);


    _tabs->setVisible(false);
    _tabs->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    layout->addWidget(_tabs, 800);
    layout->addStretch(50);

    QDialogButtonBox *btnBox = new QDialogButtonBox();
    QPushButton *restartBtn = btnBox->addButton("Redémarer", QDialogButtonBox::ActionRole);
    connect(restartBtn, SIGNAL(clicked()), this, SLOT(onRestartClicked()));
    QPushButton *cancelBtn = btnBox->addButton("Annuler", QDialogButtonBox::RejectRole);
    connect(cancelBtn, SIGNAL(clicked()), this, SIGNAL(exitDeviceSettings()));
    _okBtn = btnBox->addButton("Ok", QDialogButtonBox::ApplyRole);
    connect(_okBtn, SIGNAL(clicked()), this, SLOT(onOkClicked()));
    layout->addWidget(btnBox);
    setFocus();
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    _networkSet = dev->isConfigured();
    onDeviceListChanged();
    onCheckOk();
}

ChapiView::~ChapiView() {
    if(_networkButtonAnim != NULL){
        delete _networkButtonAnim;
    }
}

QMap<quint16, QString> ChapiView::getUnusedOutputs() const {
    if(_target == NULL){
        return QMap<quint16, QString>();
    }

    QMap<quint16, QString> outputsAll = _target->getOutputs();
    qDebug() << "targets outputs:";
    foreach(quint16 outputIndex, outputsAll.keys()){
        qDebug() << outputIndex;
    }
    qDebug() << "outputs by btn:";
    foreach(quint16 btnIndex, _outputsByBtns.keys()){
        qDebug() << btnIndex << ":" << _outputsByBtns[btnIndex];
    }

    foreach(quint16 outputIndex, _outputsByBtns){
        if(outputIndex != ChapiDevice::NO_SOURCE){
            outputsAll.remove(outputIndex);
        }
    }
    qDebug() << "unused outputs:";
    foreach(quint16 btnIndex, outputsAll.keys()){
        qDebug() << btnIndex << ":" << outputsAll[btnIndex];
    }
    return outputsAll;
}

void ChapiView::onAddBtnClicked() {
    QMap<quint16, QString> outputs = getUnusedOutputs();

    QMap<QVariant, QString> values;
    foreach(quint16 index, outputs.keys()) {
        values.insert(QVariant(index), outputs[index]);
    }

    QList<QVariant> selection = QSelectDialog::select("Ajout d'une sortie", "Choisissez la sortie", values, QList<QVariant>(), false, this);
    if(selection.isEmpty()){
        return;
    }
    quint16 index = selection.first().toUInt();


    QMap<QVariant, QString> availablesButtons;
    for(quint16 i = 0; i < _dev->nbrButtons(); i++){
        if(!_outputsByBtns.contains(i)){
            availablesButtons.insert(QVariant(i), "Bouton "+QString::number(i+1));
        }
    }
    QList<QVariant> newSelectionVar = QSelectDialog::select("Configuration de la sortie",
                "Choisissez les boutons assignés à la sortie "+outputs[index],
                availablesButtons,  QList<QVariant>(), true, this);

    if(newSelectionVar.isEmpty()){
        return;
    }

    QList<quint16> newSelection;
    foreach(QVariant var, newSelectionVar){
        newSelection.push_back(var.toUInt());
    }

    foreach(QVariant btnIndex, newSelectionVar){
        _outputsByBtns.insert(btnIndex.toUInt(), index);
    }

    bool foundFreeBtn = false;
    for(quint16 i = 0; i < _dev->nbrButtons(); i++){
        if(!_outputsByBtns.contains(i)){
            foundFreeBtn = true;
            break;
        }
    }
    _addOutputBtn->setEnabled(foundFreeBtn);

    OutputView *outView = new OutputView(index, outputs[index]);
    connect(outView, SIGNAL(outputRemove(quint16)), this, SLOT(onOutputRemoved(quint16)));
    connect(outView, SIGNAL(outputSettings(quint16)), this, SLOT(onOutputSettings(quint16)));
    _outputLayout->insertWidget(_outputLayout->count() -2, outView);
    updateInputs();
    onCheckOk();
}

void ChapiView::updateInputs(){
    while(_inputLayout->count() > 0){
        delete(_inputLayout->takeAt(0)->widget());
    }
    if(_target == NULL){
        return;
    }

    QMap<quint16, QString> outputs = _target->getOutputs();
    int nbrOutputGroups = _outputLayout->count()-2;
    for(int outputGroupIndex = 0; outputGroupIndex < nbrOutputGroups; outputGroupIndex++){
        quint16 outputIndex = dynamic_cast<OutputView*>(_outputLayout->itemAt(outputGroupIndex)->widget())->index();
        QGroupBox *group = new QGroupBox(outputs[outputIndex]);
        group->setProperty("outputIndex", QVariant(outputIndex));
        QFormLayout *layout = new QFormLayout();
        for(quint16 currentBtnIndex = 0; currentBtnIndex < _dev->nbrButtons(); currentBtnIndex++){
            if(!_outputsByBtns.contains(currentBtnIndex) || _outputsByBtns[currentBtnIndex] != outputIndex){
                continue;
            }
            QComboBox *inputCombo = new QComboBox();
            layout->addRow(new QLabel("Bouton "+QString::number(currentBtnIndex+1)+":"), inputCombo);
            inputCombo->setProperty("btnIndex", QVariant(currentBtnIndex));
        }
        group->setLayout(layout);
        _inputLayout->addWidget(group);
        updateInputGroup(group);
    }
    _inputLayout->addStretch(1);

    QList<quint16> unusedBtns;
    for(quint16 i = 0; i < _dev->nbrButtons(); i++){
        if(!_outputsByBtns.contains(i)){
            unusedBtns.push_back(i);
        }
    }
    if(unusedBtns.empty()){
        return;
    }
    QGroupBox *group = new QGroupBox("Boutons non-utilisés:");
    QVBoxLayout *layout = new QVBoxLayout();
    foreach(quint16 index, unusedBtns){
        layout->addWidget(new QLabel("Bouton "+QString::number(index+1)));
        if(_inputsByBtns.contains(index)){
            _inputsByBtns.remove(index);
        }
    }
    group->setLayout(layout);
    _inputLayout->addWidget(group);
}

void ChapiView::onBtnInputSelected(int index) {
    Q_UNUSED(index);
    QComboBox *inputCombo = ((QComboBox*)sender());
    quint16 btnIndex = inputCombo->property("btnIndex").toUInt();
    qint32 inputIndex = inputCombo->currentData().toInt();
    if(inputIndex < 0){
        _inputsByBtns.remove(btnIndex);
    }
    else{
        _inputsByBtns[btnIndex] = inputIndex;
    }
    updateInputGroup((QGroupBox*)(inputCombo->parentWidget()));
}

void ChapiView::updateInputGroup(QGroupBox *group) {
    QLayout *layout = group->layout();
    quint16 outputIndex = group->property("outputIndex").toUInt();
    for(int i = 0; i < layout->count(); i++){
        QComboBox *inputCombo = dynamic_cast<QComboBox*>(layout->itemAt(i)->widget());
        if(inputCombo == NULL){
            continue;
        }
        disconnect(inputCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onBtnInputSelected(int)));
        quint16 currentBtnIndex = inputCombo->property("btnIndex").toUInt();
        inputCombo->clear();
        QMap<quint16, QString> input = _target->getInputs();
        foreach(quint16 btnIndex, _inputsByBtns.keys()){
            if((btnIndex != currentBtnIndex) && _outputsByBtns.contains(btnIndex) && (_outputsByBtns[btnIndex] == outputIndex)){
                input.remove(_inputsByBtns[btnIndex]);
            }
        }

        inputCombo->addItem("Aucune source", QVariant((qint32)ChapiDevice::NO_SOURCE));
        foreach(quint16 index, input.keys()){
            inputCombo->addItem(input[index], QVariant((qint32)index));
        }
        inputCombo->insertItem(0, "Sélectionnez une entrée", QVariant((qint32)-2));
        if(_inputsByBtns.contains(currentBtnIndex)){
            inputCombo->setCurrentIndex(inputCombo->findData(QVariant((qint32)_inputsByBtns[currentBtnIndex])));
        }
        else {
            inputCombo->setCurrentIndex(0);
        }
        qobject_cast<QStandardItemModel *>(inputCombo->model())->item(0)->setEnabled(false);
        connect(inputCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(onBtnInputSelected(int)));
    }
}


void ChapiView::onOutputRemoved(quint16 index){
    for(int i = 0; i < _outputLayout->count() -2; i++){
        OutputView *outView = dynamic_cast<OutputView*>(_outputLayout->itemAt(i)->widget());
        if(outView->index() == index){
            _outputLayout->removeWidget(outView);
            delete outView;
        }
    }

    bool foundFreeBtn = false;
    for(quint16 i = 0; i < _dev->nbrButtons(); i++){
        if(_outputsByBtns.contains(i)){
            if(_outputsByBtns[i] == index){
                _outputsByBtns.remove(i);
                foundFreeBtn = true;
            }
        }
        else {
            foundFreeBtn = true;
        }
    }
    _addOutputBtn->setEnabled(foundFreeBtn);
    _outputLayout->parentWidget()->updateGeometry();
    _outputLayout->parentWidget()->parentWidget()->updateGeometry();
    updateInputs();
    onCheckOk();
}

void ChapiView::onOutputSettings(quint16 ouputIndex){
    if(_target == NULL){
        return;
    }
    QMap<quint16, QString> outputs = _target->getOutputs();

    QList<QVariant> selectedButtons;
    QMap<QVariant, QString> availablesButtons;
    for(quint16 btnIndex = 0; btnIndex < _dev->nbrButtons(); btnIndex++){
        if(_outputsByBtns.contains(btnIndex)){
            if(_outputsByBtns[btnIndex] != ouputIndex){
                continue;
            }
            selectedButtons.push_back(QVariant(btnIndex));
        }
        availablesButtons.insert(QVariant(btnIndex), "Bouton "+QString::number(btnIndex+1));
    }
    QList<QVariant> newSelectionVar = QSelectDialog::select("Configuration de la sortie",
                "Choisissez les boutons assignés à la sortie "+outputs[ouputIndex],
                availablesButtons, selectedButtons, true, this);

    if(newSelectionVar.isEmpty()){
        return;
    }

    QList<quint16> newSelection;
    foreach(QVariant var, newSelectionVar){
        newSelection.push_back(var.toUInt());
    }

    bool foundFreeBtn = false;
    for(quint16 btnIndex = 0; btnIndex < _dev->nbrButtons(); btnIndex++){
        if(_outputsByBtns.contains(btnIndex)){
                if((_outputsByBtns[btnIndex] == ouputIndex) && !newSelection.contains(btnIndex)){
                    _outputsByBtns.remove(btnIndex);
                    foundFreeBtn = true;
                }
        }
        else {
            if(newSelection.contains(btnIndex)){
                _outputsByBtns.insert(btnIndex, ouputIndex);
            }
            else{
                foundFreeBtn = true;
            }
        }
    }
    _addOutputBtn->setEnabled(foundFreeBtn);
    updateInputs();
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

    TargetableDevice *originalDevice = _dev->targetMac() == 0 ? NULL : _devList->targetableDeviceByMac(mac);
    if(mac != 0) {
        _targetBox->setCurrentIndex(_targetBox->findData(QVariant(mac)));
    }
    else {
        if((_target == NULL) && (originalDevice != NULL)){
            int devIndex = _targetBox->findData(_dev->targetMac());
            _targetBox->setCurrentIndex(devIndex);
            onTargetSelected(devIndex);
        }
        else{
            _targetBox->setCurrentIndex(0);
            onTargetSelected(0);
        }
    }
    qobject_cast<QStandardItemModel *>(_targetBox->model())->item(0)->setEnabled(false);
    connect(_targetBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onTargetSelected(int)));
}

void ChapiView::onTargetSelected(int index) {
    _tabs->setVisible(false);
    _okBtn->setEnabled(false);
    _outputsByBtns.clear();
    _inputsByBtns.clear();
    if(index == 0) {
        return;
    }
    quint64 mac = _targetBox->itemData(index).toULongLong();
    if(mac == 0){
        return;
    }
    TargetableDevice *newTarget = _devList->targetableDeviceByMac(mac);
    if(newTarget == NULL){
        return;
    }
    if(_target == newTarget){
        return;
    }
    _target = newTarget;
    quint16 nbrBtns = _dev->nbrButtons();
    QList<quint16> seenOutputs;
    for(quint16 i = 0; i < nbrBtns; i++){
        if(_dev->inputIndex(i) != ChapiDevice::NO_SOURCE) {
            _inputsByBtns.insert(i, _dev->inputIndex(i));
        }
        if(_dev->outputIndex(i) != ChapiDevice::NO_SOURCE) {
            _outputsByBtns.insert(i, _dev->outputIndex(i));
            if(!seenOutputs.contains(_dev->outputIndex(i))){
                seenOutputs.push_back(_dev->outputIndex(i));
            }
        }
    }

    QMap<quint16, QString> outputsNames = _target->getOutputs();
    foreach(quint16 outputIndex, seenOutputs){
        qDebug() << outputIndex;
        OutputView *outView = new OutputView(outputIndex, outputsNames[outputIndex]);
        connect(outView, SIGNAL(outputRemove(quint16)), this, SLOT(onOutputRemoved(quint16)));
        connect(outView, SIGNAL(outputSettings(quint16)), this, SLOT(onOutputSettings(quint16)));
        _outputLayout->insertWidget(_outputLayout->count() -2, outView);
    }
    _tabs->setVisible(true);
    _addOutputBtn->setEnabled(getUnusedOutputs().size() > 0);
    updateInputs();
    onCheckOk();
}

void ChapiView::onCheckOk(int unused) {
    Q_UNUSED(unused);
    _okBtn->setEnabled(false);
    if(!_networkSet){
        return;
    }
    if(!_dev->isConfigurableNow()){
        return;
    }
    if(_target == NULL){
        return;
    }
    if(_outputLayout->count() < 3){
        return;
    }
    //FIXME
    /*foreach(QComboBox *inputBox, _inputBoxes){
        if(inputBox->currentData().toInt() < 0){
            _okBtn->setEnabled(false);
            return;
        }
    }*/

    _okBtn->setEnabled(true);
}

void ChapiView::onNetworkBtnClicked() {
    NetworkSettingsView net(&_netCfg, _dev, _devList, this);
    int result = net.exec();
    if(result == QDialog::Accepted) {
        _networkSet = true;
        if(_networkButtonAnim != NULL){
            _networkButtonAnim->stop();
            delete _networkButtonAnim;
            _networkButtonAnim = NULL;
            _networkButton->setColorisation(0);
        }
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
    /*if(!_newName.isEmpty()){
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
    emit exitDeviceSettings();*/
}
