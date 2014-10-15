#include "chapiserverapp.h"

#include <winsparkle.h>

int ask_app_exit() {
    QMetaObject::invokeMethod(ChapiServerApp::instance(), "onExitAsked", Qt::QueuedConnection);
    return 0;
}

int main(int argc, char *argv[]) {
    ChapiServerApp app(argc, argv);
    if(app.isQuitting()){
        return 1;
    }
    win_sparkle_set_registry_path("Software\\Chapi Server\\updates");
    win_sparkle_set_automatic_check_for_updates(1);
    win_sparkle_set_update_check_interval(3600*24);
    win_sparkle_set_can_shutdown_callback(ask_app_exit);
    win_sparkle_init();
    int result = app.exec();
    win_sparkle_cleanup();
    return result;
}
