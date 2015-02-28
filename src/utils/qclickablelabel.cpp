#include "qclickablelabel.h"

#include <QMouseEvent>

QClickableLabel::QClickableLabel(QWidget *parent) :
    QLabel(parent)
{
}


void QClickableLabel::mouseDoubleClickEvent(QMouseEvent * event){
    QLabel::mouseDoubleClickEvent(event);
    emit doubleClick();
    event->setAccepted(true);
}
