#include "videohubview.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QInputDialog>
#include <QTabWidget>
#include <QScrollArea>

#include "../models/videohubdevice.h"
#include "../utils/qclickablelabel.h"

VideoHubView::VideoHubView(VideoHubDevice *dev, QWidget *parent) :
    QIntegratedFrame(parent)
{
    _dev = dev;

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    setMaximumWidth(450);

    QHBoxLayout *labelBox = new QHBoxLayout();
    labelBox->addStretch(1);

    QIcon icon;
    icon.addFile(QStringLiteral(":/icons/vh.png"), QSize(), QIcon::Normal, QIcon::Off);
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

    QTabWidget *tab = new QTabWidget();
    tab->setIconSize(QSize(32, 32));

    QWidget *content = new QWidget();
    QVBoxLayout *plugsLayout = new QVBoxLayout();
    QMap<quint8, QString> labels = _dev->inputLabels();
    QClickableLabel *plugLabel;
    content->setLayout(plugsLayout);
    plugsLayout->setSizeConstraint(QLayout::SetFixedSize);
    QIcon icon1;
    icon1.addFile(QStringLiteral(":/icons/input.png"), QSize(), QIcon::Normal, QIcon::Off);
    tab->addTab(content, icon1, tr("Entrées"));

    foreach(quint8 inputIndex, labels.keys()){
        plugLabel = new QClickableLabel();
        plugLabel->setProperty("vh_index", inputIndex);
        plugLabel->setText(labels[inputIndex]);
        connect(plugLabel, SIGNAL(doubleClick()), this, SLOT(onInputDoubleClick()));
        plugsLayout->addWidget(plugLabel);
    }
    plugsLayout->addStretch(1);

    content = new QWidget();
    plugsLayout = new QVBoxLayout();
    labels = _dev->outputLabels();
    content->setLayout(plugsLayout);
    plugsLayout->setSizeConstraint(QLayout::SetFixedSize);
    QIcon icon2;
    icon2.addFile(QStringLiteral(":/icons/output.png"), QSize(), QIcon::Normal, QIcon::Off);
    tab->addTab(content, icon2, tr("Sorties"));

    foreach(quint8 inputIndex, labels.keys()){
        plugLabel = new QClickableLabel();
        plugLabel->setProperty("vh_index", inputIndex);
        plugLabel->setText(labels[inputIndex]);
        connect(plugLabel, SIGNAL(doubleClick()), this, SLOT(onOutputDoubleClick()));
        plugsLayout->addWidget(plugLabel);
    }
    plugsLayout->addStretch(1);
    layout->addWidget(tab);
    layout->addStretch(2);

    QDialogButtonBox *btnBox = new QDialogButtonBox();
    QPushButton *cancelBtn = btnBox->addButton("Annuler", QDialogButtonBox::RejectRole);
    connect(cancelBtn, SIGNAL(clicked()), this, SIGNAL(exitDeviceSettings()));
    QPushButton *okBtn = btnBox->addButton("Ok", QDialogButtonBox::ApplyRole);
    connect(okBtn, SIGNAL(clicked()), this, SLOT(onOkClicked()));
    layout->addWidget(btnBox);
    setFocus();
    layout->setSizeConstraint(QLayout::SetMinimumSize);
}

void VideoHubView::onNameDoubleClick() {
    bool ok = true;
    QString text = QInputDialog::getText(this, tr("Renommer"),
                tr("Nouveau Nom:"), QLineEdit::Normal, _nameLabel->text(), &ok);
    if(ok && !text.isEmpty()) {
        _nameLabel->setText(text);
        _newName = text;
    }
}

void VideoHubView::onInputDoubleClick(){
    QClickableLabel *label = (QClickableLabel*) sender();
    quint8 index = label->property("vh_index").toUInt();
    bool ok;
    QString text = QInputDialog::getText(this, tr("Renommer l'entrée N°")+(QString::number(index+1)),
                tr("Nouveau Nom:"), QLineEdit::Normal, label->text(), &ok);
    if(ok && !text.isEmpty()) {
        if(_newInputNames.contains(index)){
            _newInputNames[index] = text;
        }
        else{
            _newInputNames.insert(index, text);
        }
        label->setText(text);
    }
}

void VideoHubView::onOutputDoubleClick(){
    QClickableLabel *label = (QClickableLabel*) sender();
    bool ok;
    quint8 index = label->property("vh_index").toUInt();
    QString text = QInputDialog::getText(this, tr("Renommer la sortie N°")+(QString::number(index+1)),
                tr("Nouveau Nom:"), QLineEdit::Normal, label->text(), &ok);
    if(ok && !text.isEmpty()) {
        if(_newOutputNames.contains(index)){
            _newOutputNames[index] = text;
        }
        else{
            _newOutputNames.insert(index, text);
        }
        label->setText(text);
    }
}


void VideoHubView::onOkClicked() {
    if(!_newName.isEmpty()){
        _dev->setName(_newName);
    }
    foreach(quint8 index, _newInputNames.keys()){
        _dev->setInputName(index, _newInputNames[index]);
    }
    foreach(quint8 index, _newOutputNames.keys()){
        _dev->setOutputName(index, _newOutputNames[index]);
    }
    emit exitDeviceSettings();
}
