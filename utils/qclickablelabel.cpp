#include "qclickablelabel.h"

QClickableLabel::QClickableLabel(QWidget *parent) :
    QLabel(parent)
{
}


void QClickableLabel::mouseDoubleClickEvent(QMouseEvent * event){
    QLabel::mouseDoubleClickEvent(event);
    emit doubleClick();
}
