#ifndef OUTPUTVIEW_H
#define OUTPUTVIEW_H

#include <QFrame>

class QPushButton;

class OutputView : public QFrame
{
    Q_OBJECT
public:
    explicit OutputView(quint16 index, const QString &name, QWidget *parent = 0);

    quint16 index() const;
    void setEnableRemoveButton(bool enabled);

private:
    quint16 _index;
    QPushButton *_removeBtn;

signals:
    void outputRemove(quint16);
    void outputSettings(quint16);

public slots:
    void onRemoveClicked();
    void onSettingsClicked();
};


#endif // OUTPUTVIEW_H
