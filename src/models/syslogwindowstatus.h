#ifndef SYSLOGWINDOWSTATUS_H
#define SYSLOGWINDOWSTATUS_H

#include <QObject>

class SyslogWindowStatus : public QObject
{
    Q_OBJECT
public:
    explicit SyslogWindowStatus(QObject *parent = 0);
    bool isDisplayed() const;
    void setHidden();
    void setDisplayed();

protected:
    bool _displayed;

signals:
    void visibilityChanged(bool);

public slots:

};

#endif // SYSLOGWINDOWSTATUS_H
