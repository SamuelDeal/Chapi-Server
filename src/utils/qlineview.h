#ifndef QLINEVIEW_H
#define QLINEVIEW_H

#include <QTableView>

class QLineView : public QTableView
{
    Q_OBJECT
public:
    explicit QLineView(QWidget *parent = 0);

protected:
    virtual void keyPressEvent(QKeyEvent * event);

private:
    void copySelection(QModelIndexList indexes);

    QAction *_copyAction;
    QAction *_copyAllAction;

public slots:
     void customMenuRequested(QPoint pos);
     void copy();
     void copyAll();
};

#endif // QLINEVIEW_H
