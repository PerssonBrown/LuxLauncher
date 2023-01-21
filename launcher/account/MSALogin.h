#ifndef LUXLAUNCHER_MSALOGIN_H
#define LUXLAUNCHER_MSALOGIN_H

#include <QtCore>
#include <QObject>
#include <QtNetwork>

struct PlayerProfile {
    QString jwtToken;
    QString name;
    QString uuid;
};

class MSALogin : public QObject{
Q_OBJECT
    QNetworkAccessManager *m_manager;
    QNetworkRequest m_request;
    QNetworkReply *m_reply;
    QEventLoop m_eventLoop;

    QString m_deviceCode;
    int m_interval;
    QString m_msAccessToken;
    QString m_xblToken;
    QString m_ush;
    QString m_xstsToken;
    QString m_jwtToken;
    QString m_uuid;
    QString m_name;
public:
    MSALogin() {
        m_manager = new QNetworkAccessManager();
    }
    QPair<QString, QString> initOAuth();
    int login();
};

#endif //LUXLAUNCHER_MSALOGIN_H
