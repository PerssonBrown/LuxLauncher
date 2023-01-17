#ifndef LUXLAUNCHER_DOWNLOADER_H
#define LUXLAUNCHER_DOWNLOADER_H

#include <QtNetwork>
#include <QtCore>

class Downloader : public QObject {
Q_OBJECT
public:
    explicit Downloader(QObject *parent = nullptr);
    void append(QUrl url);
    void append(const QStringList &urls);
    static QString getFileName(const QUrl &url);

signals:
    void finished();

private slots:
    void startNextDownload();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void downloadFinished();
    void downloadReadyRead();

private:
    bool isHttpRedirect() const;
    void reportRedirect();

    QNetworkAccessManager manager;
    QQueue<QUrl> downloadQueue;
    QNetworkReply *currentDownload = nullptr;
    QFile file;
    QElapsedTimer downloadTimer;

    int downloadedCount = 0;
    int totalCount = 0;
};

#endif //LUXLAUNCHER_DOWNLOADER_H
