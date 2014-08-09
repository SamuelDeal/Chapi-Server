#ifndef VIDEOHUBVIEW_H
#define VIDEOHUBVIEW_H

#include <QMap>

#include "../utils/qintegratedframe.h"

class VideoHubDevice;
class QClickableLabel;

class VideoHubView : public QIntegratedFrame
{
    Q_OBJECT

public:
    explicit VideoHubView(VideoHubDevice *dev, QWidget *parent = 0);

private:
    VideoHubDevice *_dev;
    QClickableLabel *_nameLabel;

    QString _newName;
    QMap<quint8, QString> _newInputNames;
    QMap<quint8, QString> _newOutputNames;

signals:

public slots:
    void onOkClicked();
    void onNameDoubleClick();
    void onInputDoubleClick();
    void onOutputDoubleClick();
};

#endif // VIDEOHUBVIEW_H
