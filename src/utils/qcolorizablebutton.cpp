#include "qcolorizablebutton.h"

#include <QGraphicsColorizeEffect>

QColorizableButton::QColorizableButton(const QString &text, QWidget *parent) :
    QPushButton(text, parent) {
    init();
}

QColorizableButton::QColorizableButton(const QIcon& icon, const QString &text, QWidget *parent) :
    QPushButton(icon, text, parent) {
    init();
}

QColorizableButton::~QColorizableButton(){
    delete _colorizeEffect;
}

void QColorizableButton::init() {
    _colorizeEffect = new QGraphicsColorizeEffect();
    _colorizeEffect->setColor(Qt::red);
    _colorisation = 0;
    _colorizeEffect->setStrength(_colorisation);
    setGraphicsEffect(_colorizeEffect);
}

qreal QColorizableButton::getColorisation() {
    return _colorisation;
}

void QColorizableButton::setColorisation(qreal val) {
    _colorisation = val;
    _colorizeEffect->setStrength(val);
    update();
}
