#ifndef LUXLAUNCHER_NETWORKREQUEST_H
#define LUXLAUNCHER_NETWORKREQUEST_H

#include <QtCore>

class NetworkRequest : public QObject {
Q_OBJECT
public:
    int sendPostRequest(const QString &strUrl, const QByteArray &data, QString &response);
    int sendPostRequest(const QString &strUrl, const QByteArray &data, const QMap<QString, QString> &header, QString &response);
    int sendGetRequest(const QString &strUrl, QString &response);
    int sendGetRequest(const QString &strUrl, const QMap<QString, QString> &header, QString &response);
};

#endif //LUXLAUNCHER_NETWORKREQUEST_H
