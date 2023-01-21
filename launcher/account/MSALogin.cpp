#include "MSALogin.h"
#include "AccountManager.h"

#include <QUrl>
#include <QtCore>
#include <QObject>
#include <QtNetwork>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonParseError>

const QString authorizationUrl = "https://login.microsoftonline.com/consumers/oauth2/v2.0/devicecode";
const QString accessTokenUrl = "https://login.microsoftonline.com/consumers/oauth2/v2.0/token";
const QString xblAuthorizationUrl = "https://user.auth.xboxlive.com/user/authenticate";
const QString xstsAuthorizationUrl = "https://xsts.auth.xboxlive.com/xsts/authorize";
const QString mcAuthorizationUrl = "https://api.minecraftservices.com/authentication/login_with_xbox";
const QString mcStoreUrl = "https://api.minecraftservices.com/entitlements/mcstore";
const QString mcProfileUrl = "https://api.minecraftservices.com/minecraft/profile";
const QString GRANT_TYPE_DEVICE = "urn:ietf:params:oauth:grant-type:device_code";
const QString MIME_TYPE_XFORM = "application/x-www-form-urlencoded";
const QString MIME_TYPE_JSON = "application/json";
const QString CLIENT_ID = "client_secret";
const QString SCOPE = "offline_access openid XboxLive.signin";

QPair<QString, QString> MSALogin::initOAuth() {
    m_request.setUrl(QUrl(authorizationUrl));
    m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                        QVariant(MIME_TYPE_XFORM));
    QString body = "client_id=" + CLIENT_ID + "&scope=" + SCOPE;
    m_reply = m_manager->post(m_request, body.toUtf8());
    QEventLoop eventLoop;
    QObject::connect(m_reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    if (m_reply->error() != QNetworkReply::NoError) {
        return QPair<QString, QString>();
    }
    QString response = QString(m_reply->readAll());
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8(), &parseError);
    QPair<QString, QString> ret;
    QJsonObject root = doc.object();
    ret.first = root.value("user_code").toString();
    ret.second = root.value("verification_uri").toString();
    m_deviceCode = root.value("device_code").toString();
    m_interval = root.value("interval").toInt();
    return ret;
}

int MSALogin::login() {
    QString body, response;
    QEventLoop eventLoop;
    QJsonParseError parseError;
    QJsonDocument doc;
    QJsonObject root;
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Step0: Authenticate microsoft account
    m_request.setUrl(QUrl(accessTokenUrl));
    m_request.setHeader(QNetworkRequest::ContentTypeHeader,
                        QVariant(MIME_TYPE_XFORM));
    body = "grant_type=" + GRANT_TYPE_DEVICE +
           "&client_id=" + CLIENT_ID +
           "&device_code=" + m_deviceCode;
    while (true) {
        m_reply = m_manager->post(m_request, body.toUtf8());
        QObject::connect(m_reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
        eventLoop.exec();
        response = QString(m_reply->readAll());
        doc = QJsonDocument::fromJson(response.toUtf8(), &parseError);
        root = doc.object();
        if (root.contains("error") == false) {
            m_msAccessToken = root.value("access_token").toString();
            break;
        }
        QString error = root.value("error").toString();
        if (error != "authorization_pending") {
            return 100;
        }
        QTimer::singleShot(m_interval * 1000, &eventLoop, SLOT(quit()));
        eventLoop.exec();
    }
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Step1: Xbox Live
    m_request.setUrl(QUrl(xblAuthorizationUrl));
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(MIME_TYPE_JSON));
    m_request.setRawHeader("Accept", MIME_TYPE_JSON.toUtf8());
    body = "{\"Properties\": {\"AuthMethod\": \"RPS\",\"SiteName\": \"user.auth.xboxlive.com\",\"RpsTicket\": \"d=" +
           m_msAccessToken + "\"},\"RelyingParty\": \"http://auth.xboxlive.com\",\"TokenType\": \"JWT\"}";
    m_reply = m_manager->post(m_request, body.toUtf8());
    QObject::connect(m_reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    if (m_reply->error() != QNetworkReply::NoError) {
        return 110;
    }
    response = QString(m_reply->readAll());
    doc = QJsonDocument::fromJson(response.toUtf8(), &parseError);
    root = doc.object();
    m_xblToken = root.value("Token").toString();
    QJsonObject DisplayClaims = root.value("DisplayClaims").toObject();
    QJsonArray xui = DisplayClaims.value("xui").toArray();
    QJsonObject xuiObject = xui.first().toObject();
    m_ush = xuiObject.value("uhs").toString();
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Step2: Obtain xsts
    m_request.setUrl(QUrl(xstsAuthorizationUrl));
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(MIME_TYPE_JSON));
    m_request.setRawHeader("Accept", MIME_TYPE_JSON);
    body = "{\"Properties\": {\"SandboxId\": \"RETAIL\",\"UserTokens\": [\"" +
            m_xblToken + "\"]},\"RelyingParty\": \"rp://api.minecraftservices.com/\",\"TokenType\": \"JWT\"}";
    m_reply = m_manager->post(m_request, body.toUtf8());
    QObject::connect(m_reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    if (m_reply->error() != QNetworkReply::NoError) {
        return 120;
    }
    response = QString(m_reply->readAll());
    doc = QJsonDocument::fromJson(response.toUtf8(), &parseError);
    root = doc.object();
    if (root.contains("XErr")) {
        return 121; // xbox account can NOT be used
    }
    m_xstsToken = root.value("Token").toString();
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Step3: Authenticate with Minecraft
    m_request.setUrl(QUrl(mcAuthorizationUrl));
    m_request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(MIME_TYPE_JSON));
    m_request.setRawHeader("Accept", MIME_TYPE_JSON.toUtf8());
    body = "{\"identityToken\": \"XBL3.0 x=" + m_ush + ";" + m_xstsToken + "\"}";
    m_reply = m_manager->post(m_request, body.toUtf8());
    QObject::connect(m_reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    if (m_reply->error() != QNetworkReply::NoError) {
        return 130;
    }
    response = QString(m_reply->readAll());
    doc = QJsonDocument::fromJson(response.toUtf8(), &parseError);
    root = doc.object();
    m_jwtToken = root.value("access_token").toString();
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Step4: Check ownership
    m_request.setUrl(QUrl(mcStoreUrl));
    QString header = "Bearer " + m_jwtToken;
    m_request.setRawHeader("Authorization", header.toUtf8());
    m_reply = m_manager->get(m_request);
    QObject::connect(m_reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    response = QString(m_reply->readAll());
    doc = QJsonDocument::fromJson(response.toUtf8(), &parseError);
    root = doc.object();
    QJsonArray items = root.value("items").toArray();
    if (items.size() == 0) {
        // account does not own minecraft
        return 141;
    }
    // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // Step5: get profiles
    m_request.setUrl(QUrl(mcProfileUrl));
    m_request.setRawHeader("Authorization", header.toUtf8());
    m_reply = m_manager->get(m_request);
    QObject::connect(m_reply, SIGNAL(finished()), &eventLoop, SLOT(quit()));
    eventLoop.exec();
    response = QString(m_reply->readAll());
    doc = QJsonDocument::fromJson(response.toUtf8(), &parseError);
    root = doc.object();
    if (root.contains("error")) {
        return 151;
    }
    m_uuid = root.value("id").toString();
    m_name = root.value("name").toString();

    PlayerProfile profile;
    profile.name = m_name;
    profile.uuid = m_uuid;
    profile.jwtToken = m_jwtToken;
    AccountManager accountManager;
    accountManager.addAccount(profile);
    return 0;
}