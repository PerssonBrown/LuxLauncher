#include "NetworkRequest.h"
#include <QUrl>
#include <QObject>
#include <QtNetwork>

int NetworkRequest::sendPostRequest(const QString &strUrl, const QByteArray &data, QString &response) {
    QMap<QString, QString> map;
    return this->sendPostRequest(strUrl, data, map, response);
}

int NetworkRequest::sendPostRequest(const QString &strUrl, const QByteArray &data, const QMap<QString, QString> &header, QString &response) {
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkRequest request;
    request.setUrl(QUrl(strUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    for (QMap<QString, QString>::const_iterator it = map.begin(); it != map.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }

    QNetworkReply *reply = manager->post(request, data);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        return statusCode.toInt();
    }
    response = QString(reply->readAll());
    return 1;
}

int NetworkRequest::sendGetRequest(const QString &strUrl, QString &response) {
    QMap<QString, QString> map;
    return this->sendGetRequest(strUrl, map, response);
}

int NetworkRequest::sendGetRequest(const QString &strUrl, const QMap<QString, QString> &header, QString &response) {
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    QNetworkRequest request;
    request.setUrl(QUrl(strUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    for (QMap<QString, QString>::const_iterator it = map.begin(); it != map.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toUtf8());
    }

    QNetworkReply *reply = manager->get(request);
    QEventLoop eventLoop;
    connect(reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
        return statusCode.toInt();
    }
    response = QString(reply->readAll());
    return 1;
}