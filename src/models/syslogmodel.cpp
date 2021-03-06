#include "syslogmodel.h"

#include <QFont>

#include "syslogger.h"
#include "device.h"

#define LIMIT 5

SyslogModel::SyslogModel() : QAbstractTableModel(0) {

}

int SyslogModel::rowCount(const QModelIndex &parent) const{
    Q_UNUSED(parent);
    return _entries.length();
}

int SyslogModel::columnCount(const QModelIndex &parent) const{
    Q_UNUSED(parent);
    return 6;
}

QVariant SyslogModel::data(const QModelIndex &index, int role) const {
    if(index.isValid() && role==Qt::FontRole) {
        return QFont("Courier New", 8);
    }
    if(!index.isValid() || (role != Qt::DisplayRole) || (index.row() < 0) || (index.row() >= _entries.length())){
        return QVariant();
    }
    switch(index.column()){
        case 0:     return _entries[index.row()].facilityName();
        case 1:     return _entries[index.row()].severityName();
        case 2:     return _entries[index.row()].date();
        case 3:     return (_entries[index.row()].host() == NULL ? "Unknown" : _entries[index.row()].host()->name());
        case 4:     return _entries[index.row()].app();
        case 5:     return _entries[index.row()].msg();
        default:    return QVariant();
    }
}

QVariant SyslogModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if((role != Qt::DisplayRole) || (orientation != Qt::Horizontal)) {
        return QVariant();
    }
    switch(section){
        case 0:     return "Facility";
        case 1:     return "Sévérité";
        case 2:     return "Date";
        case 3:     return "Périphérique";
        case 4:     return "Application";
        case 5:     return "Message";
        default:    return QVariant();
    }
}

quint8 SyslogModel::severityAt(int rowIndex) const {
    return _entries[rowIndex].severity();
}

quint64 SyslogModel::devMacAt(int rowIndex) const {
    return _entries[rowIndex].host()->mac();
}

void SyslogModel::addEntry(SyslogEntry entry) {
    int length = _entries.length();
    beginInsertRows(QModelIndex(), length, length);
    _entries.push_back(entry);
    endInsertRows();

    if(_entries.length() > LIMIT){
        beginRemoveRows(QModelIndex(), 0, 0);
        _entries.pop_front();
        endRemoveRows();
    }
}
