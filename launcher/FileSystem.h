#ifndef LUXLAUNCHER_FILESYSTEM_H
#define LUXLAUNCHER_FILESYSTEM_H

#include <QFile>
#include <QString>

class FileSystem {
public:
    FileSystem();
    bool isExists(const QString &path);
    QFile* writeOnlyPoint(const QString &path);
    QFile* readOnlyPoint(const QString &path);
    QFile* appendPoint(const QString &path);
    QString readOnly(const QString &path);
    bool writeOnly(const QString &path, const QString &str);
    bool append(const QString &path, const QString &str);
};

#endif //LUXLAUNCHER_FILESYSTEM_H
