#-------------------------------------------------
#
# Project created by QtCreator 2014-06-12T01:25:13
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChapiServer
TEMPLATE = app
RC_FILE = resources/chapi.rc
QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra -pedantic
TRANSLATIONS += resources/qt_fr.qm
win32:LIBS += -liphlpapi -lwinsparkle -lole32 -lshlwapi -luuid

win32:INCLUDEPATH += $$PWD/../WinSparkle-0.4/include
win32:LIBS += -L"$$PWD/../WinSparkle-0.4/Release"

SOURCES += \
    src/models/atemdevice.cpp \
    src/models/chapidevice.cpp \
    src/models/connecteddevice.cpp \
    src/models/device.cpp \
    src/models/devicelist.cpp \
    src/models/devicescanner.cpp \
    src/models/serverdevice.cpp \
    src/models/syslogentry.cpp \
    src/models/syslogger.cpp \
    src/models/syslogmodel.cpp \
    src/models/targetabledevice.cpp \
    src/models/videohubdevice.cpp \
    src/utils/atemprotocol.cpp \
    src/utils/errorlist.cpp \
    src/utils/netutils.cpp \
    src/utils/nlprotocol.cpp \
    src/utils/qclickablelabel.cpp \
    src/utils/qcolorizablebutton.cpp \
    src/utils/qintegratedframe.cpp \
    src/utils/qipwidget.cpp \
    src/utils/qradiobox.cpp \
    src/utils/qselectdialog.cpp \
    src/utils/sighandler.cpp \
    src/utils/syslogfilter.cpp \
    src/views/apptrayview.cpp \
    src/views/atemview.cpp \
    src/views/chapiview.cpp \
    src/views/deviceview.cpp \
    src/views/mainview.cpp \
    src/views/networksettingsview.cpp \
    src/views/nmappathview.cpp \
    src/views/outputview.cpp \
    src/views/syslogview.cpp \
    src/views/videohubview.cpp \
    src/chapiserverapp.cpp \
    src/main.cpp \
    src/utils/qlineview.cpp \
    src/models/syslogwindowstatus.cpp \
    src/models/versionlist.cpp \
    src/models/version.cpp \
    src/utils/downloader.cpp \
    src/utils/async.cpp \
    src/rsc_strings.cpp \
    src/views/versionselectorview.cpp

HEADERS  += \
    src/models/atemdevice.h \
    src/models/chapidevice.h \
    src/models/connecteddevice.h \
    src/models/device.h \
    src/models/devicelist.h \
    src/models/devicescanner.h \
    src/models/networkconfig.h \
    src/models/serverdevice.h \
    src/models/syslogentry.h \
    src/models/syslogger.h \
    src/models/syslogmodel.h \
    src/models/targetabledevice.h \
    src/models/videohubdevice.h \
    src/utils/atemprotocol.h \
    src/utils/errorlist.h \
    src/utils/infint.h \
    src/utils/netutils.h \
    src/utils/nlprotocol.h \
    src/utils/qclickablelabel.h \
    src/utils/qcolorizablebutton.h \
    src/utils/qintegratedframe.h \
    src/utils/qipwidget.h \
    src/utils/qradiobox.h \
    src/utils/qselectdialog.h \
    src/utils/sighandler.h \
    src/utils/syslogfilter.h \
    src/views/apptrayview.h \
    src/views/atemview.h \
    src/views/chapiview.h \
    src/views/deviceview.h \
    src/views/mainview.h \
    src/views/networksettingsview.h \
    src/views/nmappathview.h \
    src/views/outputview.h \
    src/views/syslogview.h \
    src/views/videohubview.h \
    src/chapiserverapp.h \
    src/const.h \
    src/utils/qlineview.h \
    src/models/syslogwindowstatus.h \
    src/models/versionlist.h \
    src/models/version.h \
    src/utils/downloader.h \
    src/utils/async.h \
    src/rsc_strings.h \
    src/views/versionselectorview.h

FORMS    +=

RESOURCES += \
    resources/resources.qrc

OTHER_FILES += \
    resources/chapi.ico \
    resources/chapi.rc
