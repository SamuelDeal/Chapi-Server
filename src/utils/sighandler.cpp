#include "sighandler.h"

#ifndef _WIN32
    #include <signal.h>
#endif //!_WIN32

SigHandler& SigHandler::get() {
    static SigHandler instance;
    return instance;
}

SigHandler::SigHandler() {
    _registeredMask = 0;
#ifdef _WIN32
    SetConsoleCtrlHandler(SigHandler::WIN32_handleFunc, TRUE);
#endif //_WIN32
}

SigHandler::~SigHandler() {
#ifdef _WIN32
    SetConsoleCtrlHandler(SigHandler::WIN32_handleFunc, FALSE);
#else
    for (int i = 0; i < NUM_SIGNALS; i++) {
        int logical = 0x1 << i;
        if (_registeredMask & logical) {
            signal(POSIX_logicalToPhysical(logical), SIG_DFL);
        }
    }
#endif //_WIN32
}

void SigHandler::addCallback(const std::function <bool (void)>&callback, SigHandler::Signal mask){
    std::lock_guard<std::mutex> lock(_mutex);
    _callbacks.push_back(std::make_pair(mask, callback));
#ifndef _WIN32
    int toRegister = _registeredMask ^ mask;
    for (int i = 0; i < NUM_SIGNALS; i++) {
        int logical = 0x1 << i;
        if (toRegister & logical) {
            struct sigaction sighandler;
            sighandler.sa_handler = POSIX_handleFunc;
            sigemptyset(&sighandler.sa_mask);
            sighandler.sa_flags = 0;
            sigaction(POSIX_logicalToPhysical(logical), &sighandler, NULL);
        }
    }
#endif // !_WIN32
    _registeredMask |= mask;
}

bool SigHandler::onSignalReceived(Signal signo) {
    bool result = false;
    std::lock_guard<std::mutex> lock(_mutex);
    for(CallbackList::iterator it = _callbacks.begin(); it != _callbacks.end(); it++){
        if(it->first & signo){
            auto callback = it->second;
            result = callback() || result;
        }
    }
    return result;
}

#ifdef _WIN32
DWORD SigHandler::WIN32_logicalToPhysical(int signal){
    switch (signal){
        case SigHandler::SIG_INT: return CTRL_C_EVENT;
        case SigHandler::SIG_TERM: return CTRL_BREAK_EVENT;
        case SigHandler::SIG_CLOSE: return CTRL_CLOSE_EVENT;
        default:
            return ~(unsigned int)0; // SIG_ERR = -1
    }
}
#else
int SigHandler::POSIX_logicalToPhysical(int signal) {
    switch (signal) {
        case SigHandler::SIG_INT: return SIGINT;
        case SigHandler::SIG_TERM: return SIGTERM;
        // In case the client asks for a SIG_CLOSE handler, accept and
        // bind it to a SIGTERM. Anyway the signal will never be raised
        case SigHandler::SIG_CLOSE: return SIGTERM;
        case SigHandler::SIG_RELOAD: return SIGHUP;
        default:
            return -1; // SIG_ERR = -1
    }
}
#endif //_WIN32


#ifdef _WIN32
int SigHandler::WIN32_physicalToLogical(DWORD signal) {
    switch (signal) {
        case CTRL_C_EVENT: return SigHandler::SIG_INT;
        case CTRL_BREAK_EVENT: return SigHandler::SIG_TERM;
        case CTRL_CLOSE_EVENT: return SigHandler::SIG_CLOSE;
        default:
            return SigHandler::SIG_UNHANDLED;
    }
}
#else
int SigHandler::POSIX_physicalToLogical(int signal) {
    switch (signal) {
        case SIGINT: return SigHandler::SIG_INT;
        case SIGTERM: return SigHandler::SIG_TERM;
        case SIGHUP: return SigHandler::SIG_RELOAD;
        default:
            return SigHandler::SIG_UNHANDLED;
    }
}
#endif //_WIN32



#ifdef _WIN32
BOOL WINAPI SigHandler::WIN32_handleFunc(DWORD signal) {
    int signo = WIN32_physicalToLogical(signal);
    if(signo == -1){
        return FALSE;
    }
    return SigHandler::get().onSignalReceived((SigHandler::Signal)signo) ? TRUE : FALSE;
}
#else
void SigHandler::POSIX_handleFunc(int signal) {
    int signo = POSIX_physicalToLogical(signal);
    if(signo == -1){
        return;
    }
    SigHandler::get().onSignalReceived((SigHandler::Signal)signo);
}
#endif //_WIN32
