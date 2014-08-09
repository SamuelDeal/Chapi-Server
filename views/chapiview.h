#ifndef CHAPIVIEW_H
#define CHAPIVIEW_H

#include "../utils/qintegratedframe.h"
#include "../models/networkconfig.h"


class ChapiDevice;
class DeviceList;
class QClickableLabel;
class QChildEvent;
class QComboBox;
class QGroupBox;
class QFormLayout;
class QSignalMapper;
class QPushButton;

class ChapiView : public QIntegratedFrame
{
    Q_OBJECT
public:
    explicit ChapiView(ChapiDevice *dev, DeviceList *devList, QWidget *parent = 0);

private:
    ChapiDevice *_dev;
    DeviceList *_devList;
    NetworkConfig _netCfg;

    QClickableLabel *_nameLabel;
    QComboBox *_targetBox;
    QComboBox *_outputBox;
    QGroupBox *_inputsBox;
    QFormLayout *_inputsLayout;
    QList<QComboBox*> _inputBoxes;
    QPushButton *_okBtn;

    quint64 _previousMacAddress;

    QString _newName;
    QString _newIp;
    QString _newMask;
    QString _newGateway;
    bool _newDhcp;
    bool _networkSet;

public slots:
    void onNetworkBtnClicked();
    void onNameDoubleClick();
    void onRestartClicked();
    void onOkClicked();
    void onTargetSelected(int);
    void onDeviceListChanged();
    void onCheckOk(int unused = 0);
};

#endif // CHAPIVIEW_H
