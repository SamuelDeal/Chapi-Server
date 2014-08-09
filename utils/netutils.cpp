#include "netutils.h"

#include <stdio.h>
#include <cmath>
#include <stdexcept>
#include <qdebug.h>

#ifdef Q_OS_WIN
#include <winsock2.h>
#include <iphlpapi.h>
#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#else
#define BUFSIZE 8192
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>
#include <unistd.h>
#endif

NetUtils::NetUtils()
{
}

QString NetUtils::macToStr(quint64 mac){
    if(mac == 0){
        return "";
    }
    QString result;
    unsigned char byteArray[6];
    byteArray[0] = (quint64)((mac >> 40) & 0XFF);
    byteArray[1] = (quint64)((mac >> 32) & 0XFF);
    byteArray[2] = (quint64)((mac >> 24) & 0xFF);
    byteArray[3] = (quint64)((mac >> 16) & 0xFF);
    byteArray[4] = (quint64)((mac >> 8) & 0XFF);
    byteArray[5] = (quint64)((mac & 0XFF));

    char buff[18];
    sprintf(buff, "%02x:%02x:%02x:%02x:%02x:%02x",
            byteArray[0], byteArray[1], byteArray[2], byteArray[3], byteArray[4], byteArray[5]);
    result.append(buff);
    return result.toUpper();
}

quint64 NetUtils::strToMac(const QString &str){
    if(str == ""){
        return 0;
    }
    unsigned char byteArray[6];
    int last = -1;
    int rc = sscanf(str.toStdString().c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx%n",
                    &byteArray[0], &byteArray[1], &byteArray[2], &byteArray[3], &byteArray[4], &byteArray[5],
                    &last);

    if(rc != 6 || str.length() != last){
        return 0;
    }

    return
        quint64(byteArray[0]) << 40 |
        quint64(byteArray[1]) << 32 |
        quint64(byteArray[2]) << 24 |
        quint64(byteArray[3]) << 16 |
        quint64(byteArray[4]) << 8 |
        quint64(byteArray[5]);
}

QHostAddress NetUtils::getNetwork(QNetworkInterface inet) {
    int i = 0;
    bool found = false;
    foreach(QNetworkAddressEntry addr,inet.addressEntries()){
        if(addr.ip().protocol() == QAbstractSocket::IPv4Protocol){
            found = true;
            break;
        }
        ++i;
    }

    if(found){
        qint32 netip = inet.addressEntries()[i].ip().toIPv4Address() & inet.addressEntries()[i].netmask().toIPv4Address();
        return QHostAddress(netip);
    }
    else{
        return QHostAddress();
    }
}

QHostAddress NetUtils::getIp(QNetworkInterface inet){
    int i = 0;
    bool found = false;
    foreach(QNetworkAddressEntry addr,inet.addressEntries()){
        if(addr.ip().protocol() == QAbstractSocket::IPv4Protocol){
            found = true;
            break;
        }
        ++i;
    }

    if(found){
        return inet.addressEntries()[i].ip();
    }
    else{
        return QHostAddress();
    }
}

int NetUtils::getMaskPrefix(QNetworkInterface inet) {
    int i = 0;
    foreach(QNetworkAddressEntry addr,inet.addressEntries()){
        if(addr.ip().protocol() == QAbstractSocket::IPv4Protocol){
            break;
        }
        ++i;
    }
    return inet.addressEntries()[i].prefixLength();
}

NetUtils::Ipv4Class NetUtils::getClass(const QHostAddress &addr) {
    quint32 ip = addr.toIPv4Address();
    if((ip & 0xa0000000) == 0) {
        return NetUtils::ClassA;
    }
    else if((ip & 0xe0000000) == 0xe0000000){
        return NetUtils::ClassDorHigher;
    }
    else if((ip & 0xc0000000) == 0xc0000000) {
        return NetUtils::ClassC;
    }
    else if((ip & 0xa0000000) == 0xa0000000) {
        return NetUtils::ClassB;
    }
    else {
        return NetUtils::InvalidClass;
    }
}

QHostAddress NetUtils::getDefaultSubmask(const QHostAddress &addr) {
    NetUtils::Ipv4Class classAddr = getClass(addr);
    switch(classAddr) {
        case NetUtils::ClassA:
            return QHostAddress("255.0.0.0");
            break;
        case NetUtils::ClassB:
            return QHostAddress("255.255.0.0");
            break;
        case NetUtils::ClassC:
            return QHostAddress("255.255.255.0");
            break;
        default:
            return QHostAddress("255.255.255.255");
            break;
    }
}

int NetUtils::ipToMaskPrefix(const QHostAddress &addr){
    if(addr.isNull() || (addr.protocol() != QAbstractSocket::IPv4Protocol)){
        return -1;
    }
    quint32 sum = 0;
    quint32 addrIp = addr.toIPv4Address();
    for(int i = 32; i > 0; --i){
        sum += pow(2, i);
        if(sum == addrIp){
            return i;
        }
    }
    return -1;
}

QHostAddress NetUtils::maskPrefixToIp(int mask){
    if(mask == -1){
        return QHostAddress();
    }
    quint32 sum = 0;
    for(int i = 32; i > (32-mask); --i){
        sum += pow(2, i);
    }
    return QHostAddress(sum);
}

QHostAddress NetUtils::getNetwork(const QHostAddress &addr, int mask) {
    return getNetwork(addr, maskPrefixToIp(mask));
}

QHostAddress NetUtils::getNetwork(const QHostAddress &addr, const QHostAddress &mask) {
    quint32 netip = addr.toIPv4Address() & mask.toIPv4Address();
    return QHostAddress(netip);
}

bool NetUtils::isNetwork(const QHostAddress &addr, int mask) {
    return isNetwork(addr, maskPrefixToIp(mask));
}

bool NetUtils::isNetwork(const QHostAddress &addr, const QHostAddress &mask) {
    return (addr.toIPv4Address() & mask.toIPv4Address()) == addr.toIPv4Address();
}

QHostAddress NetUtils::getBroadcast(const QHostAddress &addr, int mask) {
    return getBroadcast(addr, maskPrefixToIp(mask));
}

QHostAddress NetUtils::getBroadcast(const QHostAddress &addr, const QHostAddress &mask) {
    quint32 broadcastIp = addr.toIPv4Address() | (~ mask.toIPv4Address());
    return QHostAddress(broadcastIp);
}

bool NetUtils::isBroadcast(const QHostAddress &addr, int mask) {
    return isBroadcast(addr, maskPrefixToIp(mask));
}

bool NetUtils::isBroadcast(const QHostAddress &addr, const QHostAddress &mask) {
    quint32 broadcastIp = addr.toIPv4Address() | (~ mask.toIPv4Address());
    return (addr == QHostAddress(QHostAddress::Broadcast)) || ((addr.toIPv4Address() & broadcastIp) == broadcastIp);
}

bool NetUtils::inSameNetwork(const QHostAddress &addr1, const QHostAddress &addr2, int mask) {
    return getNetwork(addr1, mask) == getNetwork(addr2, mask);
}

bool NetUtils::inSameNetwork(const QHostAddress &addr1, const QHostAddress &addr2, const QHostAddress &mask) {
    return getNetwork(addr1, mask) == getNetwork(addr2, mask);
}

bool NetUtils::isValidAddress(const QHostAddress &addr){
    return (!addr.isNull() && !addr.isLoopback() && (addr != QHostAddress(QHostAddress::Broadcast)));
}

bool NetUtils::isValidAddress(const QHostAddress &addr, int mask){
    return isValidAddress(addr, maskPrefixToIp(mask));
}

bool NetUtils::isValidAddress(const QHostAddress &addr, const QHostAddress &mask){
    return (isValidAddress(addr) && !isBroadcast(addr, mask) && !isNetwork(addr, mask));
}

bool NetUtils::isValidMask(const QHostAddress &addr) {
    return ipToMaskPrefix(addr) != -1;
}


#ifndef Q_OS_WIN


struct route_info {
    in_addr dstAddr;
    in_addr srcAddr;
    in_addr gateWay;
    char ifName[IF_NAMESIZE];
};

int readNlSock(int sockFd, char *bufPtr, unsigned int seqNum, int pId) {
    struct nlmsghdr *nlHdr;
    int readLen = 0;
    int msgLen = 0;

    do {
        if((readLen = recv(sockFd, bufPtr, BUFSIZE - msgLen, 0)) < 0) {
            perror("SOCK READ: ");
            return -1;
        }
        nlHdr = (struct nlmsghdr *)bufPtr;
        if((NLMSG_OK(nlHdr, readLen) == 0) || (nlHdr->nlmsg_type == NLMSG_ERROR)) {
            perror("Error in received packet");
            return -1;
        }
        if(nlHdr->nlmsg_type == NLMSG_DONE) {
            break;
        }

        bufPtr += readLen;
        msgLen += readLen;

        if((nlHdr->nlmsg_flags & NLM_F_MULTI) == 0) {
            break;
        }
    }
    while((nlHdr->nlmsg_seq != seqNum) || (nlHdr->nlmsg_pid != (unsigned)pId));

    return msgLen;
}

void parseRoutes(struct nlmsghdr *nlHdr, struct route_info *rtInfo) {
    struct rtmsg *rtMsg = (struct rtmsg *)NLMSG_DATA(nlHdr);

    if((rtMsg->rtm_family != AF_INET) || (rtMsg->rtm_table != RT_TABLE_MAIN)) {
        return;
    }

    struct rtattr *rtAttr = (struct rtattr *)RTM_RTA(rtMsg);

    int rtLen = RTM_PAYLOAD(nlHdr);
    for (; RTA_OK(rtAttr, rtLen); rtAttr = RTA_NEXT(rtAttr, rtLen)) {
        switch (rtAttr->rta_type) {
            case RTA_OIF:
                if_indextoname(*(int *)RTA_DATA(rtAttr), rtInfo->ifName);
                break;
            case RTA_GATEWAY:
                rtInfo->gateWay = *(in_addr *)RTA_DATA(rtAttr);
                break;
            case RTA_PREFSRC:
                rtInfo->srcAddr = *(in_addr *)RTA_DATA(rtAttr);
                break;
            case RTA_DST:
                rtInfo->dstAddr = *(in_addr *)RTA_DATA(rtAttr);
                break;
            default: //do nothing
                break;
        }
    }
}
#endif

QList<QHostAddress> NetUtils::getGateways() {
    QList<QHostAddress> result;
#ifdef Q_OS_WIN
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;

    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    pAdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof(IP_ADAPTER_INFO));
    if(pAdapterInfo == NULL) {
        qDebug() << "Error allocating memory needed to call GetAdaptersinfo";
        return result;
    }

    //Make an initial call to GetAdaptersInfo to get the necessary size into the ulOutBufLen variable
    if(GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            qDebug() << "Error allocating memory needed to call GetAdaptersinfo";
            return result;
        }
    }

    dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
    if(dwRetVal != NO_ERROR) {
        qDebug() << "GetAdaptersInfo failed with error: " << dwRetVal;
    }
    else {
        pAdapter = pAdapterInfo;
        while(pAdapter) {
            QHostAddress detected(pAdapter->GatewayList.IpAddress.String);
            if((detected != QHostAddress(QHostAddress::Any)) && (detected.protocol() == QAbstractSocket::IPv4Protocol)){
                result.push_back(detected);
            }
            pAdapter = pAdapter->Next;
        }
    }
    if(pAdapterInfo){
        free(pAdapterInfo);
    }
#else
    int sock, msgSeq = 0;
    if((sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE)) < 0) {
        perror("Socket Creation: ");
        return result;
    }

    char msgBuf[BUFSIZE];
    memset(msgBuf, 0, BUFSIZE);
    struct nlmsghdr *nlMsg = (struct nlmsghdr *)msgBuf;

    nlMsg->nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg));
    nlMsg->nlmsg_type = RTM_GETROUTE;

    nlMsg->nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
    nlMsg->nlmsg_seq = msgSeq++;
    nlMsg->nlmsg_pid = getpid();

    if(send(sock, nlMsg, nlMsg->nlmsg_len, 0) < 0) {
        return result;
    }

    int len;
    if((len = readNlSock(sock, msgBuf, msgSeq, getpid())) < 0) {
        return result;
    }

    struct route_info *rtInfo = (struct route_info *)malloc(sizeof(struct route_info));

    for(; NLMSG_OK(nlMsg, len); nlMsg = NLMSG_NEXT(nlMsg, len)) {
        memset(rtInfo, 0, sizeof(struct route_info));
        parseRoutes(nlMsg, rtInfo);

        if(strstr((char *)inet_ntoa(rtInfo->dstAddr), "0.0.0.0") && !strstr((char *)inet_ntoa(rtInfo->gateWay), "0.0.0.0")) {
            char buf[64];
            inet_ntop(AF_INET, &rtInfo->gateWay, buf, sizeof(buf));
            result.push_back(QHostAddress(QString(buf)));
        }
    }

    free(rtInfo);
    close(sock);
#endif
    return result;
}












