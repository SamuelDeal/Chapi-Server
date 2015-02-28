#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QMap>
#include <QNetworkRequest>
#include <functional>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkRequest;

class Downloader : public QObject
{
    Q_OBJECT
public:
    explicit Downloader(QObject *parent = 0);
    ~Downloader();

    int download(QNetworkRequest request,
                  const std::function <void (QNetworkReply *)>&onSuccessCallback,
                  const std::function <void (const QString &)>&onErrorCallback = NULL,
                  const std::function <void (qint64, qint64)>&onProgressCallback = NULL);
    void cancelDownload(int requestId);

private:
    struct RequestDetails {
        QList<QUrl> redirectList;
        QNetworkReply *reply;
        std::function <void (QNetworkReply *)> onSuccessCallback;
        std::function <void (const QString &)> onErrorCallback;
        std::function <void (qint64, qint64)> onProgressCallback;
    };

    QNetworkAccessManager *_network;
    QMap<int, RequestDetails> _requests;

    int getNextRequestId() const;

signals:

public slots:
    void onReplyFinished(QNetworkReply* reply);
    void onProgress(qint64 bytesReceived, qint64 bytesTotal);
};

#endif // DOWNLOADER_H
