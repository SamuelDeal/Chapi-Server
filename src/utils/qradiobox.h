#ifndef QRADIOBOX_H
#define QRADIOBOX_H

#include <QGroupBox>

class QRadioBox : public QGroupBox
{
    Q_OBJECT
public:
    explicit QRadioBox(QWidget *parent = 0);
    explicit QRadioBox(const QString &title, QWidget *parent = 0);

    void paintEvent(QPaintEvent *event);
    int getLeftMargin() const;

private:
    int _leftMargin;

signals:

public slots:

};

#endif // QRADIOBOX_H
