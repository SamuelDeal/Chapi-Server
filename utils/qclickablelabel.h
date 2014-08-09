#ifndef QCLICKABLELABEL_H
#define QCLICKABLELABEL_H

#include <QLabel>

class QClickableLabel : public QLabel
{
    Q_OBJECT
public:
    explicit QClickableLabel(QWidget *parent = 0);

    void mouseDoubleClickEvent(QMouseEvent * event);

signals:
    void doubleClick();

public slots:

};

#endif // QCLICKABLELABEL_H
