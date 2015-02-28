#include "syslogentry.h"

#include <QDebug>
#include <QStringList>
#include <QDateTime>



SyslogEntry::SyslogEntry(const QStringList &entryData, Device* dev) {
    _facility = entryData[1].toUInt() / 8;
    _severity = entryData[1].toUInt() % 8;

    //ignore date, using local date (because embed device doesn't have real clock)
    _date = QDateTime::currentDateTime();
    _host = dev;

    QRegExp re("^\\s*\\w+:.*$");
    if(re.exactMatch(entryData[6])){
        _app = entryData[6].section(':', 0, 0);
        _msg = entryData[6].section(':', 1).trimmed();
    }
    else {
        _app = "";
        _msg = entryData[6].trimmed();
    }
}

SyslogEntry::SyslogEntry(QtMsgType type, const QString &msg, Device* dev) {
    _facility = 3;
    switch(type){
       case QtMsgType::QtDebugMsg:
            _severity = 7;
            break;
        case QtMsgType::QtWarningMsg:
            _severity = 4;
            break;
        case QtMsgType::QtCriticalMsg:
            _severity = 2;
            break;
        case QtMsgType::QtFatalMsg:
            _severity = 1;
            break;
        default:
            _severity = 6;
            break;
    }
    _app = "chapi server";
    _msg = msg;
    _date = QDateTime::currentDateTime();
    _host = dev;
}

quint32 SyslogEntry::facility() const {
    return _facility;
}

quint8 SyslogEntry::severity() const {
    return _severity;
}

QString SyslogEntry::facilityName() const {
    static QStringList facilities{"kern", "user", "mail", "daemon", "auth", "syslog", "lpr", "news",
                                  "uucp", "clock", "authpriv", "ftp", "log audit", "log alert", "cron",
                                  "local0", "local1", "local2", "local3", "local4", "local5", "local6", "local7"};
    if(_facility >= facilities.length()){
        return QString::number(_facility);
    }
    return facilities[_facility];
}

QString SyslogEntry::severityName() const {
    static QStringList severities{"Urgence", "Alerte", "Critique", "Erreur", "Warning", "Information", "Détail", "Debug"};
    if(_severity >= severities.length()){
        return QString::number(_severity);
    }
    return severities[_severity];
}


QDateTime SyslogEntry::date() const{
    return _date;
}

Device* SyslogEntry::host() const {
    return _host;
}

QString SyslogEntry::app() const{
    return _app;
}

QString SyslogEntry::msg() const{
    return _msg;
}
