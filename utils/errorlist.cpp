#include "errorlist.h"

#include <QMessageBox>
#include <algorithm>

ErrorList::ErrorList(const QString &title, QWidget *parentWidget) {
    _status = ErrorList::NotRunned;
    _parentWidget = parentWidget;
    _nbrCritical = 0;
    _nbrCritical = 0;
    _title = title;
}


void ErrorList::addError(bool critical, const QString &description, const ErrorCallback &onIgnore, const ErrorCallback &onAbort){
    //allow to use the same instance several times
    if(_status != ErrorList::NotRunned) {
        _errors.clear();
        _nbrCritical = 0;
        _nbrCritical = 0;
        _status = ErrorList::NotRunned;
    }

    _errors.push_back(ErrorData(critical, description, onIgnore, onAbort));
    if(critical){
        ++_nbrCritical;
    }
    else{
        ++_nbrWarnings;
    }
}

bool ErrorList::isAborted() const {
    return _status == ErrorList::Aborted;
}

void ErrorList::run(bool yesToAllEnabled) {
    if(_status != ErrorList::NotRunned) {
        return;
    }

    //start with critical errors
    _errors.sort([](const ErrorData &a, const ErrorData &b)-> bool {
        if(a.critical && !b.critical){
            return true;
        }
        return false;
    });

    unsigned int i = 0;
    bool skip = false;
    bool abort = false;
    QMessageBox::StandardButton result;
    for(std::list<ErrorData>::iterator it = _errors.begin(); it != _errors.end(); it++){
        if(!skip && !abort){
            ++i;
            QMessageBox::StandardButtons buttons = QMessageBox::Yes|QMessageBox::Abort;
            if(yesToAllEnabled && (i >= _nbrCritical) && (i != _errors.size())){
                buttons |= QMessageBox::YesToAll;
            }
            if(it->critical){
                result = QMessageBox::critical(_parentWidget, _title, it->descr, buttons, QMessageBox::Abort);
            }
            else {
                result = QMessageBox::warning(_parentWidget, _title, it->descr, buttons, QMessageBox::Yes);
            }
            if(result == QMessageBox::YesToAll) {
                skip = true;
                result = QMessageBox::Yes;
            }
        }


        if(result == QMessageBox::Abort){
            it->onAbort();
            abort = true;
            _status = ErrorList::Aborted;
        }
        else{
            it->onIgnore();
        }
    }
    if(!abort){
        _status = ErrorList::Ignored;
    }
}

ErrorList::ErrorData::ErrorData(bool isCritical, const QString &description, const ErrorCallback &lambdaIgnore, const ErrorCallback &lambdaAbort) :
    onIgnore(lambdaIgnore), onAbort(lambdaAbort) {
    critical = isCritical;
    descr = description;
}

ErrorList::ErrorData::ErrorData(const ErrorList::ErrorData &err) :
    onIgnore(err.onIgnore), onAbort(err.onAbort) {
    critical = err.critical;
    descr = err.descr;
}
