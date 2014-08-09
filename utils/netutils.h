#ifndef NETUTILS_H
#define NETUTILS_H

#include <qstring.h>
#include <qnetworkinterface.h>
#include <qhostaddress.h>


class NetUtils
{
public:
    enum Ipv4Class {
        ClassA = 1,
        ClassB,
        ClassC,
        ClassDorHigher,
        InvalidClass
    };

    static QString macToStr(quint64 mac);
    static quint64 strToMac(const QString &str);
    static QHostAddress getNetwork(QNetworkInterface inet);
    static QHostAddress getIp(QNetworkInterface inet);
    static int getMaskPrefix(QNetworkInterface inet);
    static Ipv4Class getClass(const QHostAddress &);
    static QHostAddress getDefaultSubmask(const QHostAddress &);
    static int ipToMaskPrefix(const QHostAddress &);
    static QHostAddress maskPrefixToIp(int mask);
    static QHostAddress getNetwork(const QHostAddress &addr, int mask);
    static QHostAddress getNetwork(const QHostAddress &addr, const QHostAddress &mask);
    static bool isNetwork(const QHostAddress &addr, int mask);
    static bool isNetwork(const QHostAddress &addr, const QHostAddress &mask);
    static QHostAddress getBroadcast(const QHostAddress &addr, int mask);
    static QHostAddress getBroadcast(const QHostAddress &addr, const QHostAddress &mask);
    static bool isBroadcast(const QHostAddress &addr, int mask);
    static bool isBroadcast(const QHostAddress &addr, const QHostAddress &mask);
    static bool inSameNetwork(const QHostAddress &addr1, const QHostAddress &addr2, int mask);
    static bool inSameNetwork(const QHostAddress &addr1, const QHostAddress &addr2, const QHostAddress &mask);
    static bool isValidAddress(const QHostAddress &addr);
    static bool isValidAddress(const QHostAddress &addr, int mask);
    static bool isValidAddress(const QHostAddress &addr, const QHostAddress &mask);
    static bool isValidMask(const QHostAddress &addr);
    static QList<QHostAddress> getGateways();

private:
    NetUtils();
};

#endif // NETUTILS_H
