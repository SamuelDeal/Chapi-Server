#ifndef NMAPPATHVIEW_H
#define NMAPPATHVIEW_H

#include <QMessageBox>

class NmapPathView : public QMessageBox
{
    Q_OBJECT
public:
    explicit NmapPathView(QWidget *parent);

    bool isCanceled();
    QString newPath();

private:
    QPushButton *_retryButton;
    QPushButton *_searchButton;
    QPushButton *_closeButton;
    bool _isCanceled;
    QString _newPath;

signals:

public slots:
    int exec();
};

#endif // NMAPPATHVIEW_H
