#ifndef QIPWIDGET_H
#define QIPWIDGET_H

#include <QFrame>
#include <QHostAddress>

class QLineEdit;
class QGraphicsColorizeEffect;

class QIpWidget : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(qreal colorisation READ getColorisation WRITE setColorisation)


public:
    QIpWidget(QWidget *parent = 0);

    qreal getColorisation();
    void setColorisation(qreal val);

    bool eventFilter(QObject *obj, QEvent *event);
    QHostAddress getValue();
    void setValue(const QHostAddress &addr);
    bool isEmpty();
    void setChildFocus();
    bool hasChildFocus();
    void errorEffect();

private:
    QLineEdit *_numberInputs[4];
    qreal _colorisation;
    QGraphicsColorizeEffect *_colorizeEffect;

    void moveNextLineEdit(int i);
    void movePrevLineEdit(int i);

signals:
    void signalTextChanged(QLineEdit* numberInput);
    void editingFinished();

public slots:
    void onSlotTextChanged(QLineEdit* numberInput);
    void onEditingFinished();
    void onErrorEffectFinished();
};

#endif // QIPWIDGET_H
