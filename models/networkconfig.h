#ifndef NETWORKCONFIG_H
#define NETWORKCONFIG_H

#include <QString>

struct NetworkConfig {
    bool useDHCP;
    QString ip;
    QString netmask;
    QString gateway;
};


#endif // NETWORKCONFIG_H
