#ifndef SYSLOGVIEW_H
#define SYSLOGVIEW_H

#include <QWidget>

class DeviceList;
class SyslogModel;
class SyslogFilter;
class QStandardItemModel;

class SyslogView : public QWidget
{
    Q_OBJECT
public:
    explicit SyslogView(SyslogModel *logModel, DeviceList *devList);
    ~SyslogView();

    void closeEvent(QCloseEvent *event);

private:
    SyslogModel *_logModel;
    DeviceList *_devList;
    SyslogFilter *_modelProxy;
    QStandardItemModel *_severityFilterModel;
    QStandardItemModel *_deviceFilterModel;

signals:

public slots:
    void onStart();
};

#endif // SYSLOGVIEW_H
