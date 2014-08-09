#ifndef QINTEGRATEDFRAME_H
#define QINTEGRATEDFRAME_H

#include <QFrame>

class QIntegratedFrame : public QFrame
{
    Q_OBJECT

public:
    QIntegratedFrame(QWidget *parent = 0);

protected:
    void childEvent(QChildEvent *event);
    bool eventFilter(QObject *obj, QEvent *event);

signals:
    void exitDeviceSettings();
};

#endif // QINTEGRATEDFRAME_H
