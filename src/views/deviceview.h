#ifndef DEVICEVIEW_H
#define DEVICEVIEW_H

#include <QFrame>

class Device;
class ChapiDevice;
class VideoHubDevice;
class AtemDevice;
class QClickableLabel;
class QLabel;
class QPushButton;
class QGraphicsColorizeEffect;

class DeviceView : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(qreal colorisation READ getColorisation WRITE setColorisation)

public:
    explicit DeviceView(Device *dev, QWidget *parent = 0);

    qreal getColorisation();
    void setColorisation(qreal val);

    void mouseDoubleClickEvent(QMouseEvent * event); //override

private:
    QClickableLabel *_name;
    QLabel *_statusIcon;
    QPushButton *_updateBtn;
    QPushButton *_monitorBtn;
    QPushButton *_settingsBtn;
    QPushButton *_blinkBtn;
    qreal _colorisation;
    QGraphicsColorizeEffect *_colorizeEffect;

    Device *_dev;
    bool _blinking;

signals:
    void chapiViewCmd(ChapiDevice*);
    void videoHubViewCmd(VideoHubDevice*);
    void atemViewCmd(AtemDevice*);
    void doubleClick();
    void chapiUpdateAsked(Device*);

public slots:
    void onNameDoubleClick();
    void onDevChanged();
    void onMonitorClick();
    void onUpdateClick();
    void onSettingsClick();
    void onBlinkClick();
    void onBlinkingFinished();
};

#endif // DEVICEVIEW_H
