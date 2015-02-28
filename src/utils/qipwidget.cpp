#include "qipwidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QIntValidator>
#include <QKeyEvent>
#include <QGraphicsColorizeEffect>
#include <QPropertyAnimation>
#include <QTimer>

QIpWidget::QIpWidget(QWidget *parent) : QFrame(parent)
{
    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);

    QHBoxLayout* layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    QColor color = this->palette().base().color();
    QColor disabledColor = palette().color(QPalette::Disabled, QPalette::Base);

    for(int i = 0; i != 4; ++i) {
        if(i != 0) {
            QLabel* dotLabel = new QLabel(".", this);
            dotLabel->setStyleSheet("QLabel{background: rgb("+QString::number(color.red())+", "+QString::number(color.green())+", "+QString::number(color.blue())+"); }\n"
                                    "QLabel:disabled{background: rgb("+QString::number(disabledColor.red())+", "+QString::number(disabledColor.green())+", "+QString::number(disabledColor.blue())+"); }");
            layout->addWidget(dotLabel);
            layout->setStretch(layout->count(), 0);
        }

        QLineEdit* numberInput = new QLineEdit(this);
        _numberInputs[i] = numberInput;
        numberInput->installEventFilter(this);

        layout->addWidget(numberInput);
        layout->setStretch(layout->count(), 1);

        numberInput->setFrame(false);
        numberInput->setAlignment(Qt::AlignCenter);

        QRegExp rx("^(0|[1-9]|[1-9][0-9]|1[0-9][0-9]|2([0-4][0-9]|5[0-5]))$");
        QValidator *validator = new QRegExpValidator(rx, numberInput);
        numberInput->setValidator(validator);
        connect(numberInput, SIGNAL(editingFinished()), this, SLOT(onEditingFinished()));
    }

    setMaximumWidth(this->minimumWidth());
    connect(this, SIGNAL(signalTextChanged(QLineEdit*)), this, SLOT(onSlotTextChanged(QLineEdit*)), Qt::QueuedConnection);
}

void QIpWidget::onSlotTextChanged(QLineEdit* numberInput) {
    int index = -1;
    for(unsigned int i = 0; i < 4; ++i){
        if(numberInput == _numberInputs[i]) {
            index = i;
            break;
        }
    }

    if((index == -1) || (index >= 3)){
        return;
    }
    if((numberInput->text() == "0") || ((numberInput->text().size() == 3) && (numberInput->text().size() == numberInput->cursorPosition()))) {
        _numberInputs[index+1]->setFocus();
        _numberInputs[index+1]->selectAll();
    }
}

void QIpWidget::onEditingFinished() {
    for(unsigned int i = 0; i < 4; ++i){
        if(_numberInputs[i]->hasFocus()){
            return;
        }
    }
    emit editingFinished();
}

bool QIpWidget::eventFilter(QObject *obj, QEvent *event) {
    bool result = QFrame::eventFilter(obj, event);
    if(event->type() != QEvent::KeyPress) {
        return result;
    }
    QKeyEvent* evt = dynamic_cast<QKeyEvent*>(event);
    if(!evt) {
        return result;
    }
    for(unsigned int i = 0; i < 4; ++i) {
        QLineEdit* numberInput = _numberInputs[i];
        if (numberInput != obj) {
            continue;
        }
        switch(evt->key()) {
            case Qt::Key_Left:
                if(numberInput->cursorPosition() == 0) {
                    movePrevLineEdit(i);
                }
                break;

            case Qt::Key_Right:
                if(numberInput->text().isEmpty() || (numberInput->text().size() == numberInput->cursorPosition())) {
                    moveNextLineEdit(i);
                }
                break;

            case Qt::Key_0:
                if(numberInput->text().isEmpty() || (numberInput->text() == "0")) {
                    numberInput->setText("0");
                    moveNextLineEdit(i);
                }
                emit signalTextChanged(numberInput);
                break;

            case Qt::Key_Backspace:
                if(numberInput->text().isEmpty() || (numberInput->cursorPosition() == 0)) {
                    movePrevLineEdit(i);
                }
                break;

            case Qt::Key_Comma:
            case Qt::Key_Period:
                moveNextLineEdit(i);
                break;

            default:
                emit signalTextChanged(numberInput);
                break;
        }
    }
    return result;
}

void QIpWidget::moveNextLineEdit(int i) {
    if(i+1 == 4) {
        return;
    }
    _numberInputs[i+1]->setFocus();
    _numberInputs[i+1]->setCursorPosition(0);
    _numberInputs[i+1]->selectAll();
}

void QIpWidget::movePrevLineEdit(int i) {
    if(i == 0) {
        return;
    }
    _numberInputs[i-1]->setFocus();
    _numberInputs[i-1]->setCursorPosition(_numberInputs[i-1]->text().size());
}

bool QIpWidget::isEmpty() {
    for(int i = 0; i < 4; i++){
        if(!_numberInputs[i]->text().isEmpty()){
            return false;
        }
    }
    return true;
}


QHostAddress QIpWidget::getValue() {
    if(isEmpty()){
        return QHostAddress("");
    }
    QStringList parts;
    for(int i = 0; i < 4; i++){
        QString txt = _numberInputs[i]->text();
        if(txt.isEmpty()){
            txt = "0";
        }
        parts.push_back(txt);
    }
    return QHostAddress(parts.join('.'));
}

void QIpWidget::setValue(const QHostAddress &addr) {
    if((addr.protocol() != QAbstractSocket::IPv4Protocol) || addr.isNull()){
        for(int i = 0; i < 4; i++){
            _numberInputs[i]->setText("");
        }
    }
    else {
        QStringList parts = addr.toString().split('.');
        for(int i = 0; i < 4; i++){
            _numberInputs[i]->setText(parts[i]);
        }
    }
}

void QIpWidget::setChildFocus(){
    if(hasChildFocus()){
        return;
    }
    _numberInputs[0]->setFocus();
    _numberInputs[0]->setCursorPosition(0);
    _numberInputs[0]->selectAll();
}

bool QIpWidget::hasChildFocus(){
    for(int i = 0; i < 4; i++){
        if(_numberInputs[i]->hasFocus()){
            return true;
        }
    }
    return false;
}

void QIpWidget::errorEffect(){
    _colorizeEffect = new QGraphicsColorizeEffect();
    _colorizeEffect->setColor(Qt::red);
    _colorisation = 0;
    _colorizeEffect->setStrength(_colorisation);
    setGraphicsEffect(_colorizeEffect);

    QPropertyAnimation *animation = new QPropertyAnimation(this, "colorisation");
    animation->setDuration(375);
    animation->setLoopCount(4);
    animation->setStartValue(0.0);
    animation->setEndValue(1.5);
    animation->setEasingCurve(QEasingCurve::CosineCurve);
    animation->start();

    QTimer::singleShot(1550, this, SLOT(onErrorEffectFinished()));
}

void QIpWidget::onErrorEffectFinished(){
    _colorisation = 0;
    _colorizeEffect->setStrength(0);
    update();
}


qreal QIpWidget::getColorisation() {
    return _colorisation;
}

void QIpWidget::setColorisation(qreal val) {
    _colorisation = val;
    _colorizeEffect->setStrength(val);
    update();
}
