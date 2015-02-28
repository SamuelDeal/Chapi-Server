#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QMainWindow>
#include <QMap>

class QTabWidget;
class DeviceList;
class DeviceView;
class QVBoxLayout;
class QScrollArea;
class Device;
class ChapiDevice;
class VideoHubDevice;
class AtemDevice;
class QStackedLayout;
class QBoxLayout;
class QPushButton;
class SyslogWindowStatus;
class VersionList;

class MainView : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainView(DeviceList &devList, SyslogWindowStatus&, VersionList&, QWidget *parent = 0);
    void hideInTray();

protected:
    void resizeEvent(QResizeEvent*);

private:
    void closeEvent(QCloseEvent *event);
    void hideEvent(QHideEvent *event);
    void initTabs();
    void addDev(QBoxLayout *layout, Device *dev);
    void removeDev(quint64 mac);

    DeviceList &_devList;
    SyslogWindowStatus &_syslogStatus;
    VersionList &_versionList;

    QStackedLayout *_stackedLayout;
    QTabWidget *_tabs;
    QVBoxLayout *_chapiLayout;
    QVBoxLayout *_vhLayout;
    QVBoxLayout *_atemLayout;
    QVBoxLayout *_otherLayout;
    QPushButton *_settingsBtn;
    QWidget *_mainPage;
    QAction *_showSyslogAction;
    QAction *_hideInTrayAction;
    QAction *_autoCheckUpdatesAction;
    QMenu *_settingsMenu;

    bool _exitOnClose;

    QMap<quint64, DeviceView *> _devViewList;
    QMap<QLayout*, unsigned int> _devCount;


signals:
    void syslogWindowShowCmd();
    void aboutCmd();
    void exitCmd();
    void chapiHelpCmd();
    void chapiServerHelpCmd();
    void chapiUpdateAsked(Device*);

public slots:
    void onDeviceListChanged();
    void onSettingsClick();
    void onChapiViewAsked(ChapiDevice*);
    void onVideoHubViewAsked(VideoHubDevice*);
    void onAtemViewAsked(AtemDevice*);
    void onDeviceSettingsExit();
    void onSyslogWindowVisibilityChanged(bool);
    void onHideOnTrayToogled();
    void onAutoCheckUpdateToogled();
    void onResized();
};

#endif // MAINVIEW_H
