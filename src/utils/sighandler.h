#ifndef SIGHANDLER_H
#define SIGHANDLER_H

#include <list>
#include <functional>
#include <mutex>
#ifdef _WIN32
    #include <windows.h>
#endif //_WIN32

//TODO: not tested yet !
//TODO: check other signals (especialy windows signals)
class SigHandler {
public:
    enum Signal : int {
        SIG_UNHANDLED   = 0,    // Physical signal not supported by this class
        SIG_NOOP        = 1,    // The application is requested to do a no-op (only a target that platform-specific signals map to when they can't be raised anyway)
        SIG_INT         = 2,    // Control+C (should terminate but consider that it's a normal way to do so; can delay a bit)
        SIG_TERM        = 4,    // Control+Break (should terminate now without regarding the consquences)
        SIG_CLOSE       = 8,    // Container window closed (should perform normal termination, like Ctrl^C) [Windows only; on Linux it maps to SIG_TERM]
        SIG_RELOAD      = 16,   // Reload the configuration [Linux only, physical signal is SIGHUP; on Windows it maps to SIG_NOOP]
        DEFAULT_SIGNALS = SIG_INT | SIG_TERM | SIG_CLOSE,
    };

    static SigHandler& get();

    ~SigHandler();

    void addCallback(const std::function <bool(void)>& , Signal mask = DEFAULT_SIGNALS);

private:
    static const int NUM_SIGNALS = 6;
    typedef std::list<std::pair<Signal, std::function<bool (void)> > > CallbackList;

    SigHandler();
    SigHandler(SigHandler const&); //No Implementation
    void operator=(SigHandler const&); //No Implementation

    bool onSignalReceived(Signal signo);

    int _registeredMask;
    std::mutex _mutex;
    CallbackList _callbacks;

#ifdef _WIN32
    static BOOL WINAPI WIN32_handleFunc(DWORD);
    static int WIN32_physicalToLogical(DWORD);
    static DWORD WIN32_logicalToPhysical(int);
#else //!_WIN32
    static void POSIX_handleFunc(int);
    static int POSIX_physicalToLogical(int);
    static int POSIX_logicalToPhysical(int);
#endif //_WIN32

};

#endif // SIGHANDLER_H
