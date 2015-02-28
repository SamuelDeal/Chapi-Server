#include "qintegratedframe.h"

#include <QChildEvent>
#include <QKeyEvent>
#include <QDialog>

QIntegratedFrame::QIntegratedFrame(QWidget *parent) :
    QFrame(parent)
{
    installEventFilter(this);

    setFrameStyle(QFrame::Panel | QFrame::Sunken);
    setLineWidth(1);

    setFocusPolicy(Qt::StrongFocus);

}

void QIntegratedFrame::childEvent(QChildEvent *event) {
    if(event->added()) {
        event->child()->installEventFilter(this);
    }
}

bool QIntegratedFrame::eventFilter(QObject *obj, QEvent *event) {
    bool result = QFrame::eventFilter(obj, event);
    if(event->type() != QEvent::KeyPress) {
        return result;
    }
    QKeyEvent* evt = dynamic_cast<QKeyEvent*>(event);
    if(!evt || (dynamic_cast<QDialog*>(obj) != NULL) || (evt->key() != Qt::Key_Escape)) {
        return result;
    }
    emit exitDeviceSettings();
    return result;
}
