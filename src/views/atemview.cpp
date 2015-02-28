#include "atemview.h"

#include <QVBoxLayout>
#include <QIcon>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QInputDialog>
#include <QTabWidget>
#include <QScrollArea>
#include <QFile>

#include "../models/AtemDevice.h"
#include "../utils/qclickablelabel.h"

AtemView::AtemView(AtemDevice *dev, QWidget *parent) :
    QIntegratedFrame(parent)
{
    _dev = dev;

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    setMaximumWidth(450);

    QHBoxLayout *labelBox = new QHBoxLayout();
    labelBox->addStretch(1);

    QIcon icon;
    icon.addFile(QStringLiteral(":/icons/imgs/atem.png"), QSize(), QIcon::Normal, QIcon::Off);
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
    QFile styleFile(":/styles/tabs.qss");
    styleFile.open(QFile::ReadOnly );
    QString style(styleFile.readAll() );
    tab->setStyleSheet(style);


    QScrollArea* scrollArea = new QScrollArea();
    QWidget *content = new QWidget();
    QVBoxLayout *plugsLayout =  new QVBoxLayout();
    content->setLayout(plugsLayout);
    content->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QIcon icon1;
    icon1.addFile(QStringLiteral(":/icons/imgs/input.png"), QSize(), QIcon::Normal, QIcon::Off);
    tab->addTab(scrollArea, icon1, tr("Entrées"));
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(content);
    scrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QMap<quint16, QString> labels = _dev->getInputs();
    QClickableLabel *plugLabel;
    foreach(quint16 inputIndex, labels.keys()){
        plugLabel = new QClickableLabel();
        plugLabel->setProperty("vh_index", inputIndex);
        plugLabel->setText(labels[inputIndex]);
        connect(plugLabel, SIGNAL(doubleClick()), this, SLOT(onInputDoubleClick()));
        plugsLayout->addWidget(plugLabel);
    }
    plugsLayout->addStretch(1);

    scrollArea = new QScrollArea();
    content = new QWidget();
    plugsLayout =  new QVBoxLayout();
    content->setLayout(plugsLayout);
    content->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    QIcon icon2;
    icon2.addFile(QStringLiteral(":/icons/imgs/output.png"), QSize(), QIcon::Normal, QIcon::Off);
    tab->addTab(scrollArea, icon2, tr("Sorties"));
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(content);
    scrollArea->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    labels = _dev->getOutputs();
    QLabel *outputLabel;
    foreach(quint16 inputIndex, labels.keys()){
        outputLabel = new QLabel();
        outputLabel->setProperty("atem_index", inputIndex);
        outputLabel->setText(labels[inputIndex]);
        plugsLayout->addWidget(outputLabel);
    }
    plugsLayout->addStretch(1);
    layout->addWidget(tab, 20);
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

void AtemView::onNameDoubleClick() {
    bool ok = true;
    QString text = QInputDialog::getText(this, tr("Renommer"),
                tr("Nouveau Nom:"), QLineEdit::Normal, _nameLabel->text(), &ok);
    if(ok && !text.isEmpty()) {
        _nameLabel->setText(text);
        _newName = text;
    }
}

void AtemView::onInputDoubleClick(){
    QClickableLabel *label = (QClickableLabel*) sender();
    quint16 index = label->property("atem_index").toUInt();
    if(index > 4096) { //special input
        return;
    }

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

void AtemView::onOkClicked() {
    if(!_newName.isEmpty()){
        _dev->setName(_newName);
    }
    foreach(quint16 index, _newInputNames.keys()){
        _dev->setInputName(index, _newInputNames[index]);
    }
    emit exitDeviceSettings();
}
