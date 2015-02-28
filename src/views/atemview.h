#ifndef ATEMVIEW_H
#define ATEMVIEW_H

#include <QMap>

#include "../utils/qintegratedframe.h"

class AtemDevice;
class QClickableLabel;

class AtemView : public QIntegratedFrame
{
    Q_OBJECT

public:
    explicit AtemView(AtemDevice *dev, QWidget *parent = 0);

private:
    AtemDevice *_dev;
    QClickableLabel *_nameLabel;

    QString _newName;
    QMap<quint16, QString> _newInputNames;

signals:

public slots:
    void onOkClicked();
    void onNameDoubleClick();
    void onInputDoubleClick();
};

#endif // ATEMVIEW_H
