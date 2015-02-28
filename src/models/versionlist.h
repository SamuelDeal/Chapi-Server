#ifndef CHAPIVERSIONLIST_H
#define CHAPIVERSIONLIST_H

#include <QObject>
#include <QMap>
#include <QNetworkRequest>

#include "version.h"
#include "../utils/downloader.h"

class QNetworkAccessManager;
class QNetworkReply;
class QJsonObject;
class QTimer;


class VersionList : public QObject
{
    Q_OBJECT
public:
    static int getAutoCheckDelay();

    explicit VersionList();
    void start();
    void checkNew();
    bool autocheck() const;
    void setAutocheck(bool value = true);
    void load();
    void save();
    QList<Version*> getAll();

    void onAutoCheckDefined();
    void removeOldVersions(QList<Version> serverVersions);
private:
    Downloader _downloader;
    QTimer *_checkTimer;

    QList<Version> _versions;
    QList<Version> _serverVersions;

    bool _autoCheckUpdates;
    bool _serverLoading;
    bool _started;
    bool _localLoaded;
    bool _shouldLoadServer;

    void onLocalLoaded();
    void saveConfigEntries();

    bool isValidReleaseJson(const QJsonObject& release) const;
    bool isValidAssetJson(const QJsonObject& release) const;
    void onReleaseListFetched(QNetworkReply* reply);
    void onVersionDownloaded(Version &version, QNetworkReply* reply);
    void downloadVersion(Version &version);
    void computeHash(Version &version, bool localVersion);
    void onLoaded(bool localLoading);
    void onVersionChecked(Version &version, bool localVersion);
    void onServerLoaded();

signals:
    void newVersionAvailable(Version*);
    void newVersionsAvailables(quint32);

public slots:
    void onAutoCheckTimer();
};

#endif // CHAPIVERSIONLIST_H
