#ifndef QCOLORIZABLEBUTTON_H
#define QCOLORIZABLEBUTTON_H

#include <QPushButton>

class QGraphicsColorizeEffect;

class QColorizableButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(qreal colorisation READ getColorisation WRITE setColorisation)

public:
    explicit QColorizableButton(const QString &text, QWidget *parent=0);
    QColorizableButton(const QIcon& icon, const QString &text, QWidget *parent=0);
    ~QColorizableButton();

    qreal getColorisation();
    void setColorisation(qreal val);

private:
    void init();

    qreal _colorisation;
    QGraphicsColorizeEffect *_colorizeEffect;

signals:

public slots:

};

#endif // QCOLORIZABLEBUTTON_H
