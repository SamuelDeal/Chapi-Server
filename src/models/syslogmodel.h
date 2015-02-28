#ifndef SYSLOGMODEL_H
#define SYSLOGMODEL_H

#include <QAbstractTableModel>

#include "syslogentry.h"

class SysLogger;

class SyslogModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit SyslogModel();

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    quint8 severityAt(int rowIndex) const;
    quint64 devMacAt(int rowIndex) const;

    void addEntry(SyslogEntry);

private:
    QList<SyslogEntry> _entries;

public slots:

};

#endif // SYSLOGMODEL_H
