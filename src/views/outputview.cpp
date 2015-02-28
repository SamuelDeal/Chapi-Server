#include "outputview.h"

#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>

OutputView::OutputView(quint16 index, const QString &name, QWidget *parent) :
    QFrame(parent)
{
    _index = index;
    QColor color = palette().window().color();
    setStyleSheet("QFrame {background: rgb("+QString::number(color.red())+", "+QString::number(color.green())+", "+QString::number(color.blue())+"); }");

    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setLineWidth(1);

    QHBoxLayout *hlayout = new QHBoxLayout();
    setLayout(hlayout);

    QLabel *label = new QLabel(name);
    hlayout->addWidget(label, 1);

    QIcon settingsIcon;
    settingsIcon.addFile(QStringLiteral(":/icons/imgs/settings.png"), QSize(), QIcon::Normal, QIcon::Off);
    QPushButton *settingsBtn = new QPushButton(settingsIcon, "");
    settingsBtn->setToolTip("Supprimer");
    connect(settingsBtn, SIGNAL(clicked()), this, SLOT(onSettingsClicked()));
    hlayout->addWidget(settingsBtn);

    QIcon removeIcon;
    removeIcon.addFile(QStringLiteral(":/icons/imgs/remove.png"), QSize(), QIcon::Normal, QIcon::Off);
    _removeBtn = new QPushButton(removeIcon, "");
    _removeBtn->setToolTip("Supprimer");
    connect(_removeBtn, SIGNAL(clicked()), this, SLOT(onRemoveClicked()));
    hlayout->addWidget(_removeBtn);

    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

quint16 OutputView::index() const {
    return _index;
}

void OutputView::onRemoveClicked() {
    emit outputRemove(_index);
}

void OutputView::onSettingsClicked() {
    emit outputSettings(_index);
}

void OutputView::setEnableRemoveButton(bool enabled) {
    _removeBtn->setEnabled(enabled);
}
