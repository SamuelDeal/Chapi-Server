#include "versionlist.h"

#ifdef _WIN32
#include <winsparkle.h>
#endif

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrl>
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QSet>
#include <QTimer>

#include "../rsc_strings.h"
#include "version.h"
#include "../utils/async.h"

#define NUMBER_OF_VERSION_TO_KEEP 3

int VersionList::getAutoCheckDelay() {
    return 3600;
}

VersionList::VersionList() :
    QObject(0) {
    _localLoaded = false;
    _shouldLoadServer = false;

    _started = false;
    _serverLoading = false;
    _checkTimer = NULL;
}

void VersionList::start() {
    if(_started){
        return;
    }
    _started = true;
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);
    _autoCheckUpdates = settings.value("Misc/auto_check_updates", true).toBool();
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/versions");
    if (!dir.exists()) {
        dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+"/versions");
    }
    load();
    onAutoCheckDefined();
}

void VersionList::load() {
    qDebug() << "loading local versions";
    start();
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/versions.ini", QSettings::IniFormat);
    QStringList versionGroups = settings.childGroups();

    _versions.clear();
    QStringList configEntryToRemove;
    foreach(QString versionGroup, versionGroups) {
        settings.beginGroup(versionGroup);
        QString name = settings.value("name", "").toString();
        int releaseId = settings.value("release_id", 0).toInt();
        QString downloadUrl = settings.value("download_url", "").toString();
        int assetId = settings.value("asset_id", 0).toInt();
        QString checksum = settings.value("checksum", "").toString();
        int assetSize = settings.value("asset_size", 0).toInt();

        if((!Version::isValidName(name)) || (releaseId <= 0) || downloadUrl.isEmpty() || (assetId <= 0) || (assetSize <= 1000)) {
            continue;
        }
        Version newVersion(name, releaseId, downloadUrl, assetId, assetSize);
        newVersion.setSavedChecksum(checksum);
        _versions.push_back(newVersion);
        settings.endGroup();
    }
    qSort(_versions);

    QStringList toKeep;
    unsigned int localVersionFiles = 0;
    for(Version &version : _versions){
        QFile file(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/versions/" +version.versionName()+".iso");
        version.setFileStatus(file.exists() ? Version::ok : Version::noFile);
        if(file.exists()){
            ++localVersionFiles;
            toKeep.append(version.versionName()+".iso");
            computeHash(version, true);
        }
    }

    saveConfigEntries(); //remove bad entries from config

    QDir versionFolder(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+ "/versions");
    QStringList filesToRemove = versionFolder.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    for(QString &filename : filesToRemove){
        if(!toKeep.contains(filename, Qt::CaseInsensitive)){
            QFileInfo info(QStandardPaths::writableLocation(QStandardPaths::DataLocation)+ "/versions/"+filename);
            if(info.isDir()){
                QDir dir(info.absoluteFilePath());
                dir.removeRecursively();
            }
            else if(info.isFile()){
                QFile::remove(info.absoluteFilePath());
            }
        }
    }
    if(localVersionFiles == 0) {
        onLocalLoaded();
    }
}

void VersionList::onLocalLoaded() {
    _localLoaded = true;



    if(_shouldLoadServer){
         checkNew();
    }
}

void VersionList::save() {
    if(!_localLoaded) {
        return;
    }
    qDebug() << "SAVING";
    saveConfigEntries();
}

void VersionList::saveConfigEntries(){
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/versions.ini", QSettings::IniFormat);
    foreach(QString key, settings.allKeys()) {
        if(key.startsWith("Version_")) {
            settings.remove(key);
        }
    };
    foreach(const Version &version, _versions) {
        settings.beginGroup("Version_"+version.versionName());
        settings.setValue("name", version.name());
        settings.setValue("release_id", QString::number(version.releaseId()));
        settings.setValue("download_url", version.downloadUrl());
        settings.setValue("asset_id", QString::number(version.assetId()));
        settings.setValue("checksum", version.localChecksum().isEmpty() ? version.savedChecksum() : version.localChecksum());
        settings.setValue("asset_size", QString::number(version.assetSize()));
        settings.endGroup();
    }
}

void VersionList::checkNew() {
    start();
    if(_serverLoading){
        return;
    }
    if(!_localLoaded){
        _shouldLoadServer = true;
        return;
    }

    qDebug() << "loading server versions";
    _serverLoading = true;
    _serverVersions.clear();
    QNetworkRequest request;
    request.setUrl(QUrl(RscStrings::chapiserver_release_url));
    request.setRawHeader("Accept", "application/vnd.github.v3+json");
    _downloader.download(request, [&](QNetworkReply *reply){
        onReleaseListFetched(reply);
        delete reply;
    }, [&](const QString &error){
        qWarning() << "Unable to retrieve release list:" << error;
        onLoaded(false);
    });
}

void VersionList::removeOldVersions(QList<Version> serverVersions) {
    bool found;
    QList<Version*> toRemove;
    for(Version &oldVersion : _versions){
        for(Version& newVersion: serverVersions) {
            if(newVersion.versionName() == oldVersion.versionName()){
                if(newVersion == oldVersion){
                    found = true;
                }
            }
        }
        if(!found){
            toRemove.append(&oldVersion);
        }
    }
    for(Version *oldVersion : toRemove){
        if(oldVersion->fileExists()){
            QFile::remove(oldVersion->path());
        }
        _versions.removeOne(*oldVersion);
    }
}

void VersionList::onReleaseListFetched(QNetworkReply* reply) {
    qDebug() << "onReleaseListFetched ok" << (quint64) reply;
    QJsonParseError jsonError;
    QJsonDocument jdoc= QJsonDocument::fromJson(reply->readAll(),&jsonError);
    if(jsonError.error != QJsonParseError::NoError){
        qWarning() << "bad json: " << jsonError.errorString();
        onLoaded(false);
        return;
    }
    if(!jdoc.isArray()){
        qWarning() << "response is not an array: " << jdoc;
        onLoaded(false);
        return;
    }
    QJsonArray releases = jdoc.array();
    QSet<QString> names;
    QList<Version> serverVersions;
    for(auto&& releaseIt: releases) {
        const QJsonObject& release = releaseIt.toObject();
        if(!isValidReleaseJson(release) && !names.contains(release["name"].toString())){
            continue;
        }
        const QJsonObject *goodAsset = NULL;
        QJsonArray assets = release["assets"].toArray();
        for(auto&& assetIt: assets) {
            const QJsonObject& asset = assetIt.toObject();
            if(isValidAssetJson(asset)){
                goodAsset = &asset;
                break;
            }
        }
        if(goodAsset == NULL){
            continue;
        }
        const QJsonObject& asset = *goodAsset;
        serverVersions.append(Version(release["name"].toString(), release["id"].toDouble(),
                asset["browser_download_url"].toString(), asset["id"].toDouble(),
                asset["size"].toDouble()));
        names.insert(release["name"].toString());
    }

    removeOldVersions(serverVersions);

    Version *lastVersion = NULL;
    if(_versions.length() > 0){
        lastVersion = &_versions.last();
    }

    QList<Version*> toAdd;
    quint32 newVersionCount = 0;
    for(Version &newVersion : serverVersions){
        bool found = false;
        for(Version& oldVersion: _versions) {
            if(newVersion == oldVersion){
                found = true;
                break;
            }
        }
        if(!found){
            toAdd.append(&newVersion);
            if((lastVersion == NULL) || (*lastVersion < newVersion)){
                ++newVersionCount;
            }
            else {
                qDebug() << "Last: " << lastVersion->versionName();
                qDebug() << "New: " << newVersion.versionName();
                qDebug() << "*lastVersion < newVersion: " << (*lastVersion < newVersion);
            }
        }
    }

    for(Version *newVersion : toAdd){
        _versions.append(*newVersion);
    }
    qSort(_versions);
    saveConfigEntries();

    if(newVersionCount == 1){
        qDebug() << "NEW VERSION SENT !!!";
        emit newVersionAvailable(&_versions.last());
    }
    else if(newVersionCount > 1){
        qDebug() << "NEW VERSION SENT !!!";
        emit newVersionsAvailables(newVersionCount);
    }
}

void VersionList::onVersionDownloaded(Version &version, QNetworkReply* reply) {
    if(reply->bytesAvailable() != version.assetSize()){
        qWarning() << "bad download length: got" << reply->bytesAvailable() << "expecting" << version.assetSize();
        downloadVersion(version);
        return;
    }

    QNetworkReply* myreply = reply;
    Async::run([&, reply]{
        QByteArray data = reply->readAll();
        QFile file(version.path());
        if(!file.open(QIODevice::WriteOnly)){
            throw AsyncError("unable open path " + version.path());
        }
        file.write(data);
        file.close();
    }, [&, reply]{
        computeHash(version, false);
        delete reply;
    }, [&, reply](QString errorMsg){
        qWarning() << "Unable to save donloaded version: " << errorMsg;
        delete reply;
        downloadVersion(version);
    });
}

void VersionList::computeHash(Version &version, bool localVersion) {
    Async::run([&, localVersion]{
        QFile file(version.path());
        if(!file.open(QIODevice::ReadOnly)){
            throw AsyncError("unable to read file " + version.path());
        }
        QByteArray data = file.readAll();
        if(data.length() < 1024){
            throw AsyncError("too small file, length = " + QString::number(data.length()) + " for file " + version.path());
        }
        QByteArray fileHash = QCryptographicHash::hash(data, QCryptographicHash::Md5);
        version.setLocalChecksum(QString(fileHash.toHex()));
        if(version.localChecksum() != version.savedChecksum() && !version.savedChecksum().isEmpty() && localVersion) {
            qWarning() << "Corrupted local file "+version.path();
            QFile::remove(version.path());
            version.setFileStatus(Version::noFile);
            version.setLocalChecksum("");
        }
        else {
            version.setFileStatus(Version::ok);
        }
    }, [&, localVersion]{
        onVersionChecked(version, localVersion);
    }, [&, localVersion](QString errorMsg){
        qWarning() << "Unable to compute version checksum:" << errorMsg;
        QFile::remove(version.path());
        version.setFileStatus(Version::noFile);
        onVersionChecked(version, localVersion);
    });
}

void VersionList::onVersionChecked(Version &checkedVersion, bool localVersion) {
    qDebug() << "Version " << checkedVersion.name() << (localVersion ? "local" : "server") << "checked";
    QList<Version> *list;
    if(localVersion){
        list = &_versions;
    }
    else{
        list = &_serverVersions;
    }
    for(Version &version : *list){
        if(version.localChecksum().isEmpty() && version.isValid() && version.fileStatus() == Version::ok){
            return; //not all are checksumed
        }
    }
    if(localVersion){
        onLocalLoaded();
    }
    else{
        onServerLoaded();
    }
}

void VersionList::onServerLoaded(){

}

void VersionList::onLoaded(bool localLoading){
    qDebug() << (localLoading ? "local" : "server") << "versions loaded!";
    QList<Version> *list;
    if(localLoading){
        list = &_versions;
    }
    else{
        list = &_serverVersions;
    }
    if(!localLoading) {
        _serverLoading = false;
    }
    QList<Version*> toRemove;
    for(Version &version : *list){
        if(!version.isValid()){
            if(version.fileExists()){
                QFile::remove(version.path());
            }
            toRemove.append(&version);
        }
    }
    for(Version *version : toRemove){
        list->removeAll(*version);
    }
    toRemove.clear();
}


void VersionList::downloadVersion(Version &version) {
    if(version.downloadAttemp() > 3){
        qWarning() << "Unable to download release" << version.name() << "(too many failure)";
        version.setInvalid();
        onVersionChecked(version, false);
        return;
    }
    version.setDownloadAttempt(version.downloadAttemp()+1);

    QNetworkRequest request(QUrl(version.downloadUrl()));
    qDebug() << "downloading " << version.downloadUrl();
    _downloader.download(request, [&](QNetworkReply *reply){
        onVersionDownloaded(version, reply);
    }, [&](const QString &error){
        downloadVersion(version);
    });
}


bool VersionList::isValidReleaseJson(const QJsonObject& release) const {
    if(!Version::isValidName(release["name"].toString())) {
        return false;
    }
    if((!release["draft"].isBool()) || release["draft"].toBool()){
        return false;
    }
    if(!release["id"].isDouble() || (release["id"].toDouble() <= 0)){
        return false;
    }
    if(!release["assets"].isArray()){
        return false;
    }
    return true;
}

bool VersionList::isValidAssetJson(const QJsonObject& asset) const {
    if(asset["name"].toString() != "image.iso") {
        return false;
    }
    if(asset["state"].toString() != "uploaded") {
        return false;
    }
    if(asset["url"].toString().isEmpty()) {
        return false;
    }
    if(!asset["size"].isDouble() || (asset["size"].toDouble() <= 0)){
        return false;
    }
    if(asset["browser_download_url"].toString().isEmpty()){
        return false;
    }
    if(!asset["id"].isDouble() || (asset["id"].toDouble() <= 0)){
        return false;
    }
    return true;
}

bool VersionList::autocheck() const {
    return _autoCheckUpdates;
}

void VersionList::onAutoCheckDefined() {
#ifdef _WIN32
    win_sparkle_set_automatic_check_for_updates(_autoCheckUpdates ? 1 : 0);
#endif
    if(_autoCheckUpdates){
        checkNew();
        if(_checkTimer == NULL){
            _checkTimer = new QTimer(this);
            connect(_checkTimer, SIGNAL(timeout()), this, SLOT(onAutoCheckTimer()));
            _checkTimer->start(getAutoCheckDelay()*1000);
        }
    }
    else if(_checkTimer != NULL) {
        delete _checkTimer;
        _checkTimer = NULL;
    }
}

void VersionList::setAutocheck(bool value) {
    _autoCheckUpdates = value;
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/settings.ini", QSettings::IniFormat);
    settings.setValue("Misc/auto_check_updates", _autoCheckUpdates);
    onAutoCheckDefined();
}

void VersionList::onAutoCheckTimer() {
    checkNew();
}

QList<Version*> VersionList::getAll() {
    QList<Version *> results;
    for(Version &version : _versions){
        results.append(&version);
    }
    return results;
}

/*
void VersionList::onHashCompleted(int assetId, QString hash, bool justDownloaded){
    if(justDownloaded){
        for(Version &version : _serverVersions){
            if(version.assetId() != assetId) {
                continue;
            }
            version.setLocalChecksum(hash);
        }
    }
    else{
        for(Version &version : _versions){
            if(version.assetId() != assetId) {
                continue;
            }
            version.setLocalChecksum(hash);
            if(!version.savedChecksum().isEmpty() && (version.savedChecksum() != version.localChecksum())){
                QFile::remove(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/versions/" +version.versionName()+".iso");
                _versions.removeAll(version);
                break;
            }
            else{
                version.setSavedChecksum(hash);
            }
        }

    }

    bool foundIncomplete = false;
    for(Version &version : _versions){
        QFileInfo checkFile(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/versions/" +version.versionName()+".iso");
        if(!checkFile.exists()) {
            continue;
        }
        if(version.localChecksum().isEmpty()){
            foundIncomplete = true;
            break;
        }
    }
    for(Version &version : _versions){
        QFileInfo checkFile(QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/versions/" +version.versionName()+".iso");
        if(!checkFile.exists()) {
            continue;
        }
        if(version.localChecksum().isEmpty()){
            foundIncomplete = true;
            break;
        }
    }
    if(!foundIncomplete && _loadedFromServer){
        _allChecksumCompleted = true;
        _versions = _versions;
        save();
    }
}
*/

