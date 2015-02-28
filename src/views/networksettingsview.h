#ifndef NETWORKSETTINGSVIEW_H
#define NETWORKSETTINGSVIEW_H

#include <QDialog>

class QRadioButton;
class QRadioBox;
class QIpWidget;
class NetworkConfig;
class DeviceList;
class Device;

class NetworkSettingsView : public QDialog
{
    Q_OBJECT
public:
    explicit NetworkSettingsView(NetworkConfig *cfg, Device *dev, DeviceList &devList, QWidget *parent = 0);

private:
    NetworkConfig *_cfg;
    Device *_dev;
    DeviceList &_devList;

    QRadioButton *_dhcpButton;
    QRadioBox *_staticIpBox;

    QIpWidget *_ip;
    QIpWidget *_mask;
    QIpWidget *_gateway;

signals:

public slots:
    void onViewInited();
    void onStaticIpToggled(bool);
    void onDhcpToggled(bool);
    void onOkClicked();
    void onCancelClicked();
    void onIpDefined();
};



#endif // NETWORKSETTINGSVIEW_H
