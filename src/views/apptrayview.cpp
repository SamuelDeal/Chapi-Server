#include "apptrayview.h"

#include <qapplication.h>
#include <qmenu.h>
#include <qdebug.h>

AppTrayView::AppTrayView() :
    QObject(NULL), _tray(this)
{
    _tray.setIcon(QIcon(":/icons/imgs/chapi.png"));
    _tray.show();
    connect(&_tray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(onIconActivated(QSystemTrayIcon::ActivationReason)));

    _toggleVisibilityAction = new QAction(tr("&Masquer"), this);
    connect(_toggleVisibilityAction, SIGNAL(triggered()), this, SLOT(onToggleVisibilityClicked()));

    QAction *quitAction = new QAction(tr("&Quitter"), this);
    connect(quitAction, SIGNAL(triggered()), this, SIGNAL(exitCmd()));

    _trayIconMenu = new QMenu("Chapi Server");
    _trayIconMenu->addAction(_toggleVisibilityAction);
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

void AppTrayView::onToggleVisibilityClicked() {
    if(_mainWindowIsVisible) {
        emit mainWindowHideCmd();
    }
    else {
        emit mainWindowShowCmd();
    }
}
