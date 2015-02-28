#include "downloader.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#define REDIRECTION_LIMIT 10

Downloader::Downloader(QObject *parent) : QObject(parent)
{
    _network = new QNetworkAccessManager(this);
    connect(_network, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReplyFinished(QNetworkReply*)));
}

Downloader::~Downloader() {
    disconnect(_network, SIGNAL(finished(QNetworkReply*)), this, SLOT(onReplyFinished(QNetworkReply*)));
    for(int requestId : _requests.keys()){
        RequestDetails details = _requests[requestId];
        _requests.remove(requestId);
        details.reply->abort();
        qDebug() << "delete 00";
        delete details.reply;
    }
}

int Downloader::getNextRequestId() const {
    if(_requests.size() >= QNetworkRequest::UserMax - 3 ){
        return -1;
    }
    for(int i = 1; i < (int)QNetworkRequest::UserMax; i++){
        if(_requests.contains(i)){
            continue;
        }
        return i;
    }
    return -1;
}

int Downloader::download(QNetworkRequest request,
              const std::function <void (QNetworkReply *)>&onSuccessCallback,
              const std::function <void (const QString &)>&onErrorCallback,
              const std::function <void (qint64, qint64)>&onProgressCallback) {

    int requestId = getNextRequestId();
    if(requestId < 1){
        if(onErrorCallback != NULL){
            onErrorCallback("too many requests already running");
        }
        return -1;
    }

    request.setAttribute(QNetworkRequest::User, QVariant(requestId));
    _requests[requestId] = RequestDetails();
    _requests[requestId].redirectList.append(request.url());
    _requests[requestId].onSuccessCallback = onSuccessCallback;
    _requests[requestId].onErrorCallback = onErrorCallback;
    _requests[requestId].onProgressCallback = onProgressCallback;
    _requests[requestId].reply = _network->get(request);
    if(onProgressCallback != NULL){
       connect(_requests[requestId].reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onProgress(qint64,qint64)));
    }
    return requestId;
}

void Downloader::cancelDownload(int requestId) {
    if((requestId < 1) || (!_requests.contains(requestId))){
        return;
    }
    RequestDetails details = _requests[requestId];
    _requests.remove(requestId);
    details.reply->abort();
    qDebug() << "delete 01";
    delete details.reply;
}

void Downloader::onProgress(qint64 bytesReceived, qint64 bytesTotal) {
    QNetworkReply *reply = (QNetworkReply *)sender();
    int requestId = reply->request().attribute(QNetworkRequest::User, QVariant(0)).toInt();
    if((requestId < 1) || (!_requests.contains(requestId))){
        qDebug() << "delete 02";
        delete reply;
        return;
    }
    RequestDetails &details = _requests[requestId];
    if(details.onProgressCallback != NULL){
        details.onProgressCallback(bytesReceived, bytesTotal);
    }
}

void Downloader::onReplyFinished(QNetworkReply* reply) {
    int requestId = reply->request().attribute(QNetworkRequest::User, QVariant(0)).toInt();
    if((requestId < 1) || (!_requests.contains(requestId))){
        qDebug() << "delete 03";
        delete reply;
        return;
    }

    RequestDetails &details = _requests[requestId];
    QVariant possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    QUrl newUrl = possibleRedirectUrl.toUrl();
    if(!newUrl.isEmpty()){
        if(details.redirectList.contains(newUrl)) {
            if(details.onErrorCallback != NULL) {
                details.onErrorCallback("redirection loop");
            }
            _requests.remove(requestId);
            qDebug() << "delete 04";
            delete reply;
            return;
        }
        if(details.redirectList.length() > REDIRECTION_LIMIT) {
            if(details.onErrorCallback != NULL) {
                details.onErrorCallback("too many redirections");
            }
            _requests.remove(requestId);
            qDebug() << "delete 05";
            delete reply;
            return;
        }
        details.redirectList.append(possibleRedirectUrl.toUrl());
        QNetworkRequest newRequest(reply->request());
        newRequest.setUrl(newUrl);
        newRequest.setAttribute(QNetworkRequest::User, QVariant(requestId));
        details.reply = _network->get(newRequest);
        if(details.onProgressCallback != NULL){
           connect(_requests[requestId].reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onProgress(qint64,qint64)));
        }
        qDebug() << "delete 06 (redirect) " << (quint64) reply;
        delete reply;
        return;
    }

    bool keepReply = false;
    if(reply->error() != QNetworkReply::NoError){
        if(details.onErrorCallback != NULL) {
            details.onErrorCallback(reply->errorString());
        }
    }
    else {
        int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        if(statusCode != 200){
            if(details.onErrorCallback != NULL) {
                details.onErrorCallback("http error (error code "+QString::number(statusCode)+")");
            }
        }
        else if(details.onSuccessCallback != NULL) {
            details.onSuccessCallback(reply);
            keepReply = true;
        }
    }
    _requests.remove(requestId);
    if(!keepReply){
        qDebug() << "delete 07";
        delete reply;
    }
}
