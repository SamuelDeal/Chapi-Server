#-------------------------------------------------
#
# Project created by QtCreator 2014-06-12T01:25:13
#
#-------------------------------------------------

QT       += core gui xml network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ChapiServer
TEMPLATE = app
RC_FILE = chapi.rc
QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra -pedantic
CONFIG += static
TRANSLATIONS = qt_fr.qm
win32:LIBS += -liphlpapi

SOURCES += main.cpp\
    models/devicelist.cpp \
    models/device.cpp \
    utils/netutils.cpp \
    models/devicescanner.cpp \
    models/chapidevice.cpp \
    models/atemdevice.cpp \
    models/videohubdevice.cpp \
    views/apptrayview.cpp \
    chapiserverapp.cpp \
    views/nmappathview.cpp \
    views/mainview.cpp \
    views/deviceview.cpp \
    utils/qclickablelabel.cpp \
    views/chapiview.cpp \
    views/networksettingsview.cpp \
    utils/qipwidget.cpp \
    utils/qradiobox.cpp \
    utils/sighandler.cpp \
    utils/errorlist.cpp \
    models/serverdevice.cpp \
    models/connecteddevice.cpp \
    utils/nlprotocol.cpp \
    views/videohubview.cpp \
    utils/qintegratedframe.cpp \
    models/targetabledevice.cpp \
    utils/atemprotocol.cpp \
    views/outputview.cpp \
    utils/qselectdialog.cpp \
    views/atemview.cpp \
    utils/qcolorizablebutton.cpp \
    models/syslogger.cpp \
    models/syslogentry.cpp \
    views/syslogview.cpp \
    models/syslogmodel.cpp \
    utils/syslogfilter.cpp

HEADERS  += \
    models/devicelist.h \
    models/device.h \
    utils/netutils.h \
    models/devicescanner.h \
    models/chapidevice.h \
    models/atemdevice.h \
    models/videohubdevice.h \
    views/apptrayview.h \
    chapiserverapp.h \
    views/nmappathview.h \
    const.h \
    views/mainview.h \
    views/deviceview.h \
    utils/qclickablelabel.h \
    views/chapiview.h \
    views/networksettingsview.h \
    utils/qipwidget.h \
    utils/qradiobox.h \
    models/networkconfig.h \
    utils/sighandler.h \
    utils/errorlist.h \
    models/serverdevice.h \
    models/connecteddevice.h \
    utils/nlprotocol.h \
    views/videohubview.h \
    utils/qintegratedframe.h \
    models/targetabledevice.h \
    utils/atemprotocol.h \
    views/outputview.h \
    utils/qselectdialog.h \
    views/atemview.h \
    utils/qcolorizablebutton.h \
    models/syslogger.h \
    models/syslogentry.h \
    views/syslogview.h \
    utils/infint.h \
    models/syslogmodel.h \
    utils/syslogfilter.h

FORMS    +=

RESOURCES += \
    rsc/resources.qrc

OTHER_FILES += \
    chapi.rc
