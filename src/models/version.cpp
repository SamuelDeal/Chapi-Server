#include "version.h"

#include <QRegExp>
#include <QStandardPaths>
#include <QFile>

Version::Version(const QString &name, int releaseId, const QString &downloadUrl, int assetId, int assetSize)
{
    _name = name;
    _releaseId = releaseId;
    _downloadUrl = downloadUrl;
    _assetId = assetId;
    _assetSize = assetSize;
    _fileStatus = Version::noFile;
    _downloadAttemp = 0;
    _valid = true;

    _major = 0;
    _medium = 0;
    _minor = 0;
    _build = 0;

    QRegExp re("v?(\\d+)\\.(\\d+)(:?\\.(\\d+))?(:?\\.(\\d+))?");
    if(_name.contains(re)){
        _major = re.cap(1).toInt();
        _medium = re.cap(2).toInt();
        if(re.captureCount() > 2){
            _minor = re.cap(3).toInt();
        }
        if(re.captureCount() > 3){
            _build = re.cap(4).toInt();
        }
    }
}

QString Version::versionName() const {
    return QString::number(_major)+"."+QString::number(_medium)+"."+QString::number(_minor)+"."+QString::number(_build);
}

bool Version::isValidName(const QString &name){
    QRegExp re("v?\\d+(\\.\\d+){1,3}");
    return re.exactMatch(name);
}

const QString& Version::downloadUrl() const {
    return _downloadUrl;
}

const QString& Version::name() const {
    return _name;
}

int Version::fileLength() const {
    return _assetSize;
}

Version::FileStatus Version::fileStatus() const {
    return _fileStatus;
}

void Version::setFileStatus(Version::FileStatus status) {
    _fileStatus = status;
}

int Version::releaseId() const {
    return _releaseId;
}

int Version::assetId() const {
    return _assetId;
}

const QString& Version::localChecksum() const {
    return _localChecksum;
}

void Version::setLocalChecksum(const QString&value) {
    _localChecksum = value;
}

const QString& Version::savedChecksum() const {
    return _savedChecksum;
}

void Version::setSavedChecksum(const QString&value) {
    _savedChecksum = value;
}

int Version::assetSize() const {
    return _assetSize;
}

int Version::downloadAttemp() const {
    return _downloadAttemp;
}

void Version::setDownloadAttempt(int value) {
    _downloadAttemp = value;
}

bool Version::isValid() const{
    return _valid;
}

void Version::setInvalid() {
    _valid = false;
}

bool Version::operator<(const Version &other) const{
    if(_major > other._major){
        return false;
    }
    if(_medium > other._medium){
        return false;
    }
    if(_minor > other._minor){
        return false;
    }
    return (_major < other._major) || (_medium < other._medium) || (_minor < other._minor) || (_build < other._build);
}

QString Version::path() const {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation) + "/versions/" +name()+".iso";
}

bool Version::fileExists() const {
    QFile file(path());
    return file.exists();
}

bool Version::operator==(const Version &other) const{
    return (_major == other._major) && (_medium == other._medium) && (_minor == other._minor) && (_build == other._build) &&
            (_releaseId == other._releaseId) && (_assetId == other._assetId) && (_assetSize == other._assetSize);
}
