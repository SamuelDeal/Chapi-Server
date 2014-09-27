#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QWidget>
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

class MainView : public QWidget
{
    Q_OBJECT
public:
    explicit MainView(DeviceList *devList, QWidget *parent = 0);
//    ~MainView();

private:
    void closeEvent(QCloseEvent *event);
    void initTabs();
    void addDev(QBoxLayout *layout, Device *dev);
    void removeDev(quint64 mac);

    QStackedLayout *_stackedLayout;
    QTabWidget *_tabs;
    QVBoxLayout *_chapiLayout;
    QVBoxLayout *_vhLayout;
    QVBoxLayout *_atemLayout;
    QVBoxLayout *_otherLayout;

    QMap<quint64, DeviceView *> _devViewList;
    QMap<QLayout*, unsigned int> _devCount;
    DeviceList *_devList;

signals:

public slots:
    void onDeviceListChanged();
    void onChapiViewAsked(ChapiDevice*);
    void onVideoHubViewAsked(VideoHubDevice*);
    void onAtemViewAsked(AtemDevice*);
    void onDeviceSettingsExit();
};

#endif // MAINVIEW_H
