#include "qradiobox.h"

#include <QStylePainter>
#include <QStyleOptionGroupBox>

QRadioBox::QRadioBox(QWidget *parent) :
    QGroupBox(parent)
{
    _leftMargin = 0;
    this->update();
}

QRadioBox::QRadioBox(const QString &title, QWidget *parent) :
    QGroupBox(title, parent)
{
    _leftMargin = 0;
    this->update();
}


void QRadioBox::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QStylePainter paint(this);
    QStyleOptionGroupBox option;
    initStyleOption(&option);
    // don't remove the original check box control, as we want to keep
    // it as a placeholder
    //  option.subControls &= ~QStyle::SC_GroupBoxCheckBox;
    paint.drawComplexControl(QStyle::CC_GroupBox, option);

    // re-use the style option, it contians enough info to make sure the
    // button is correctly checked
    option.rect = style()->subControlRect(QStyle::CC_GroupBox, &option, QStyle::SC_GroupBoxCheckBox, this);
    option.rect.setRect(option.rect.x()-1,option.rect.y()-1,option.rect.width()+2, option.rect.height()+2);

    // now erase the checkbox
    paint.save();
    //QScreen::grabWindow(WId window, int x = 0, int y = 0, int width = -1, int height = -1)

    QPixmap px(option.rect.width(), option.rect.height());
    px.fill(palette().window().color());
    QBrush brush(px);
    paint.fillRect(option.rect, brush);
    paint.restore();

    // and replace it with a radio button
    paint.drawPrimitive(QStyle::PE_IndicatorRadioButton, option);
    _leftMargin = option.rect.left() +1;
}

int QRadioBox::getLeftMargin() const {
    return _leftMargin;
}
