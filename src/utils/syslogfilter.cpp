#include "syslogfilter.h"

#include "../models/syslogmodel.h"

SyslogFilter::SyslogFilter(SyslogModel &logModel) :
    QSortFilterProxyModel(0), _logModel(logModel) {
    setSourceModel(&_logModel);
}

bool SyslogFilter::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const{
    Q_UNUSED(sourceParent);
    return _acceptedSeverities.contains(_logModel.severityAt(sourceRow)) && _acceptedDevices.contains(_logModel.devMacAt(sourceRow));
}

void SyslogFilter::severitySelectionChanged(QModelIndex a, QModelIndex b){
    Q_UNUSED(b);
    updateSeverityFilter(dynamic_cast<const QStandardItemModel*>(a.model()));
    invalidateFilter();
}

void SyslogFilter::deviceSelectionChanged(QModelIndex a, QModelIndex b){
    Q_UNUSED(b);
    updateDeviceFilter(dynamic_cast<const QStandardItemModel*>(a.model()));
    invalidateFilter();
}

void SyslogFilter::setInitialFilter(const QStandardItemModel* serverity, const QStandardItemModel* device) {
    updateSeverityFilter(serverity);
    updateDeviceFilter(device);
    invalidateFilter();
}

void SyslogFilter::updateSeverityFilter(const QStandardItemModel* serverity) {
    _acceptedSeverities.clear();
    for(int row = 0; row < serverity->rowCount(); row++) {
        if(serverity->item(row)->checkState()){
            _acceptedSeverities.append(serverity->item(row)->data(Qt::UserRole).toUInt());
        }
    }
}

void SyslogFilter::updateDeviceFilter(const QStandardItemModel* device) {
    _acceptedDevices.clear();
    for(int row = 0; row < device->rowCount(); row++) {
        if(device->item(row)->checkState()){
            _acceptedDevices.append(device->item(row)->data(Qt::UserRole).toULongLong());
        }
    }
}
