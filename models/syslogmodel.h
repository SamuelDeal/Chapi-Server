#ifndef SYSLOGMODEL_H
#define SYSLOGMODEL_H

#include <QAbstractTableModel>

#include "syslogentry.h"

class SysLogger;

class SyslogModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SyslogModel(SysLogger *sysloger);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    quint8 severityAt(int rowIndex) const;
    quint64 devMacAt(int rowIndex) const;

private:
    SysLogger *_logger;
    QList<SyslogEntry> _entries;

public slots:
    void onNewEntry(SyslogEntry);
};

#endif // SYSLOGMODEL_H
