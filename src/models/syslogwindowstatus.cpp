#include "syslogwindowstatus.h"

SyslogWindowStatus::SyslogWindowStatus(QObject *parent) :
    QObject(parent)
{
    _displayed = false;
}

bool SyslogWindowStatus::isDisplayed() const {
    return _displayed;
}

void SyslogWindowStatus::setHidden(){
    _displayed = false;
    emit visibilityChanged(false);
}

void SyslogWindowStatus::setDisplayed() {
    _displayed = true;
    emit visibilityChanged(true);
}
