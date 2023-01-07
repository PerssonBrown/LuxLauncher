#include "filesystem.h"
#include <QFile>

Filesystem::Filesystem() {}

bool Filesystem::isExists(const QString &path) {
    if (path.isEmpty() || !QFile::exists(path)) {
        return false;
    }
    return true;
}

QFile* Filesystem::readOnlyPoint(const QString &path) {
    if (!this->isExists(path)) {
        return nullptr;
    }
    QFile *ptr = new QFile(path);
    if (!ptr->open(QIODevice::ReadOnly)) {
        return nullptr;
    }
    return ptr;
}

QFile* Filesystem::writeOnlyPoint(const QString &path) {
    if (!this->isExists(path)) {
        return nullptr;
    }
    QFile *ptr = new QFile(path);
    if (!ptr->open(QIODevice::WriteOnly)) {
        return nullptr;
    }
    return ptr;
}

QFile* Filesystem::appendPoint(const QString &path) {
    if (!this->isExists(path)) {
        return nullptr;
    }
    QFile *ptr = new QFile(path);
    if (!ptr->open(QIODevice::WriteOnly | QIODevice::Append)) {
        return nullptr;
    }
    return ptr;
}

QString Filesystem::readOnly(const QString &path) {
    QFile *file = this->readOnlyPoint(path);
    if (file == nullptr) {
        return QString();
    }
    return file->readAll();
}

bool Filesystem::writeOnly(const QString &path, const QString &str) {
    QFile *file = this->writeOnlyPoint(path);
    if (file == nullptr) {
        return false;
    }
    QByteArray strBytes = str.toUtf8();
    file->write(strBytes, strBytes.length());
    file->close();
    return true;
}

bool Filesystem::append(const QString &path, const QString &str) {
    QFile *file = this->appendPoint(path);
    if (file == nullptr) {
        return false;
    }
    QByteArray strBytes = str.toUtf8();
    file->write(strBytes, strBytes.length());
    file->close();
    return true;
}