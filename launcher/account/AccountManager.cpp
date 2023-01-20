#include "AccountManager.h"
#include "../FileSystem.h"
#include "MSALogin.h"

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

AccountManager::AccountManager() {
    m_storagePath = QDir::currentPath() + "/playerProfiles.json";
}

void AccountManager::addAccount(PlayerProfile player) {
    FileSystem fs;
    if (QFile::exists(m_storagePath) == false) {
        fs.writeOnly(m_storagePath, "{\"profiles\": []}");
    }
    QString originStr = fs.readOnly(m_storagePath);
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(originStr.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        return;
    }

    QJsonObject root = doc.object();
    QJsonArray profilesArray = root.value("profiles").toArray();

    QJsonObject account;
    account.insert("name", QJsonValue(player.name));
    account.insert("uuid", QJsonValue(player.uuid));
    account.insert("access_token", QJsonValue(player.jwtToken));

    profilesArray.insert(profilesArray.begin(), account);
    QJsonObject newRoot;
    newRoot.insert("profiles", profilesArray);

    QJsonDocument newDoc;
    newDoc.setObject(newRoot);
    QString newStr = newDoc.toJson();
    fs.writeOnly(m_storagePath, newStr);
}