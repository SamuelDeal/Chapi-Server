#ifndef VERSION_H
#define VERSION_H

#include <QString>

class Version
{
public:
    enum FileStatus {
        noFile,
        toRemove,
        ok
    };

    Version(const QString &name, int releaseId, const QString &downloadUrl, int assetId, int assetSize);

    FileStatus fileStatus() const;
    void setFileStatus(FileStatus);
    const QString& localChecksum() const;
    void setLocalChecksum(const QString&);
    const QString& savedChecksum() const;
    void setSavedChecksum(const QString&);

    const QString &name() const;
    QString versionName() const;
    const QString &downloadUrl() const;
    QString path() const;
    bool fileExists() const;
    int fileLength() const;
    int releaseId() const;
    int assetId() const;
    int assetSize() const;

    int downloadAttemp() const;
    void setDownloadAttempt(int value);

    bool isValid() const;
    void setInvalid();

    bool operator<(const Version&) const;
    bool operator==(const Version&) const;

    static bool isValidName(const QString &name);

private:
    FileStatus _fileStatus;
    QString _localChecksum;
    QString _savedChecksum;

    QString _name;
    int _releaseId;
    QString _downloadUrl;
    int _assetId;    
    int _assetSize;
    bool _valid;
    int _downloadAttemp;

    int _major;
    int _medium;
    int _minor;
    int _build;
};

#endif // VERSION_H
