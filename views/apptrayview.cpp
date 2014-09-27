#include "apptrayview.h"

#include <qapplication.h>
#include <qmenu.h>
#include <qdebug.h>

AppTrayView::AppTrayView() :
    QObject(NULL), _tray(this)
{
    _tray.setIcon(QIcon(":/icons/chapi.png"));
    _tray.show();
    connect(&_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onIconActivated(QSystemTrayIcon::ActivationReason)));

    _toggleVisibilityAction = new QAction(tr("&Masquer"), this);
    connect(_toggleVisibilityAction, SIGNAL(triggered()), this, SLOT(onToggleVisibilityClicked()));

    _showSyslogAction = new QAction(tr("&Logs"), this);
    connect(_showSyslogAction, SIGNAL(triggered()), this, SIGNAL(syslogWindowShowCmd()));

    QAction *aboutAction = new QAction(tr("&A propos"), this);
    connect(aboutAction, SIGNAL(triggered()), this, SIGNAL(aboutCmd()));

    QAction *quitAction = new QAction(tr("&Quitter"), this);
    connect(quitAction, SIGNAL(triggered()), this, SIGNAL(exitCmd()));

    _trayIconMenu = new QMenu("Chapi Server");
    _trayIconMenu->addAction(_toggleVisibilityAction);
    _trayIconMenu->addAction(_showSyslogAction);
    _trayIconMenu->addAction(aboutAction);
    _trayIconMenu->addSeparator();
    _trayIconMenu->addAction(quitAction);

    _tray.setContextMenu(_trayIconMenu);
    _tray.setToolTip("Chapi Serveur");
}

void AppTrayView::onIconActivated(QSystemTrayIcon::ActivationReason reason){
    switch(reason){
        case QSystemTrayIcon::Context:
            //do nothing;
            break;

        case QSystemTrayIcon::Trigger:
            if(_mainWindowIsVisible){
                _trayIconMenu->popup(QCursor::pos());
            }
            else {
                emit mainWindowShowCmd();
            }
            break;

        case QSystemTrayIcon::Unknown:
        case QSystemTrayIcon::DoubleClick:
        case QSystemTrayIcon::MiddleClick:
        default:
            displayTooltip();
    }
}

void AppTrayView::displayTooltip(){
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::Information;
    _tray.showMessage("Chapi server", "Le serveur Chapi continuera à tourner en tâche de fond", icon, 1000);
}

void AppTrayView::onMainWindowVisibilityChanged(bool visible) {
    _mainWindowIsVisible = visible;
    _toggleVisibilityAction->setText(visible ? tr("&Masquer") : tr("&Afficher"));
}

void AppTrayView::onSyslogWindowVisibilityChanged(bool visible) {
    _showSyslogAction->setEnabled(!visible);
}

void AppTrayView::onToggleVisibilityClicked() {
    if(_mainWindowIsVisible) {
        emit mainWindowHideCmd();
    }
    else {
        emit mainWindowShowCmd();
    }
}
