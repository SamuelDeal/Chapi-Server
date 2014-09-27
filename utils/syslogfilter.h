#ifndef SYSLOGFILTER_H
#define SYSLOGFILTER_H

#include <QSortFilterProxyModel>
#include <QStandardItemModel>


class SyslogModel;

class SyslogFilter : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SyslogFilter(SyslogModel *logModel);

    void setInitialFilter(const QStandardItemModel* serverity, const QStandardItemModel* device);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const;
    void updateSeverityFilter(const QStandardItemModel* serverity);
    void updateDeviceFilter(const QStandardItemModel* device);

private:
    SyslogModel *_logModel;
    QList<quint8> _acceptedSeverities;
    QList<quint64> _acceptedDevices;

signals:

public slots:
    void severitySelectionChanged(QModelIndex,QModelIndex);
    void deviceSelectionChanged(QModelIndex,QModelIndex);
};

#endif // SYSLOGFILTER_H
