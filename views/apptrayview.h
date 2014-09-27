#ifndef APPTRAYVIEW_H
#define APPTRAYVIEW_H

#include <QObject>
#include <qsystemtrayicon.h>
#include <qaction.h>


class AppTrayView : public QObject
{
    Q_OBJECT
public:
    explicit AppTrayView();

    void displayTooltip();

private:
    QSystemTrayIcon _tray;
    bool _mainWindowIsVisible;
    QAction *_toggleVisibilityAction;
    QAction *_showSyslogAction;
    QMenu *_trayIconMenu;

signals:
    void mainWindowShowCmd();
    void mainWindowHideCmd();
    void syslogWindowShowCmd();
    void aboutCmd();
    void exitCmd();

public slots:
    void onToggleVisibilityClicked();
    void onIconActivated(QSystemTrayIcon::ActivationReason);
    void onMainWindowVisibilityChanged(bool);
    void onSyslogWindowVisibilityChanged(bool);
};

#endif // APPTRAYVIEW_H
