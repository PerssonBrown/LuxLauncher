#ifndef LUXLAUNCHER_ACCOUNTMANAGER_H
#define LUXLAUNCHER_ACCOUNTMANAGER_H

#include "MSALogin.h"

#include <QtCore>

class AccountManager {
    QString m_storagePath;

public:
    AccountManager();
    void addAccount(PlayerProfile player);
};

#endif //LUXLAUNCHER_ACCOUNTMANAGER_H
