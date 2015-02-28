#include "chapiserverapp.h"

#include <QSettings>
#include <QStandardPaths>

#ifdef _WIN32
#include <winsparkle.h>
#endif

#include "rsc_strings.h"
#include "models/versionlist.h"

int ask_app_exit() {
    QMetaObject::invokeMethod(ChapiServerApp::instance(), "onExitAsked", Qt::QueuedConnection);
    return 0;
}

int main(int argc, char *argv[]) {
    ChapiServerApp app(argc, argv);
    if(app.isQuitting()){
        return 1;
    }
#ifdef _WIN32
    win_sparkle_set_appcast_url(RscStrings::appcast_release_url);
    win_sparkle_set_registry_path("Software\\Chapi Server\\updates");
    win_sparkle_set_automatic_check_for_updates(0);
    win_sparkle_set_update_check_interval(VersionList::getAutoCheckDelay());
    win_sparkle_set_can_shutdown_callback(ask_app_exit);
    win_sparkle_init();
#endif
    int result = app.exec();
#ifdef _WIN32
    win_sparkle_cleanup();
#endif
    return result;
}
