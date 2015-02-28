#ifndef CHAPIVIEW_H
#define CHAPIVIEW_H

#include <QMap>

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
class QVBoxLayout;
class QTabWidget;
class TargetableDevice;
class QPropertyAnimation;
class QColorizableButton;

class ChapiView : public QIntegratedFrame
{
    Q_OBJECT
public:
    explicit ChapiView(ChapiDevice *dev, DeviceList &devList, QWidget *parent = 0);
    ~ChapiView();

private:
    QMap<quint16, QString> getUnusedOutputs() const;
    void updateInputs();
    void updateInputGroup(QGroupBox *group);

    DeviceList &_devList;
    ChapiDevice *_dev;    
    NetworkConfig _netCfg;

    QClickableLabel *_nameLabel;
    QComboBox *_targetBox;
    QTabWidget *_tabs;
    QVBoxLayout *_outputLayout;
    QVBoxLayout *_inputLayout;
    QVBoxLayout *_faderLayout;
    QPushButton *_addOutputBtn;
    QColorizableButton *_networkButton;

    QPushButton *_okBtn;

    QString _newName;
    QString _newIp;
    QString _newMask;
    QString _newGateway;
    TargetableDevice *_target;
    QMap<quint16, quint16> _outputsByBtns;
    QMap<quint16, quint16> _inputsByBtns;
    bool _newDhcp;
    bool _networkSet;
    QPropertyAnimation *_networkButtonAnim;

public slots:
    void onNetworkBtnClicked();
    void onNameDoubleClick();
    void onRestartClicked();
    void onOkClicked();
    void onTargetSelected(int);
    void onDeviceListChanged();
    void onCheckOk(int unused = 0);
    void onAddBtnClicked();
    void onOutputRemoved(quint16);
    void onOutputSettings(quint16);
    void onBtnInputSelected(int);
};

#endif // CHAPIVIEW_H
