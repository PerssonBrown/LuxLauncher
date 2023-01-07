#include "downloader.h"

Downloader::Downloader(QObject *parent) : QObject(parent) {}

void Downloader::append(const QStringList &urls)
{
    for (const QString &urlAsString: urls) {
        append(QUrl::fromEncoded(urlAsString.toLocal8Bit()));
    }
    if (downloadQueue.isEmpty()) {
        QTimer::singleShot(0, this, &Downloader::finished);
    }
}

void Downloader::append(QUrl url)
{
    if (downloadQueue.isEmpty()) {
        QTimer::singleShot(0, this, &Downloader::startNextDownload);
    }
    downloadQueue.enqueue(url);
    ++totalCount;
}

QString Downloader::getFileName(const QUrl &url) {
    QString path = url.path();
    QString basename = QFileInfo(path).fileName();
    if (basename.isEmpty()) {
        basename = "download";
    }
    if (QFile::exists(basename)) {
        int i = 0;
        basename += '.';
        while (QFile::exists(basename + QString::number(i))) {
            ++i;
        }
        basename += QString::number(i);
    }
    return basename;
}

void Downloader::startNextDownload()
{
    if (downloadQueue.isEmpty()) {
        printf("%d/%d files downloaded successfully\n", downloadedCount, totalCount);
        emit finished();
        return;
    }

    QUrl url = downloadQueue.dequeue();

    QString filename = getFileName(url);
    file.setFileName(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        fprintf(stderr, "Problem opening save file '%s' for download '%s': %s\n",
                qPrintable(filename), url.toEncoded().constData(),
                qPrintable(file.errorString()));

        startNextDownload();
        return;
    }

    QNetworkRequest request(url);
    currentDownload = manager.get(request);
    connect(currentDownload, &QNetworkReply::downloadProgress,
            this, &Downloader::downloadProgress);
    connect(currentDownload, &QNetworkReply::finished,
            this, &Downloader::downloadFinished);
    connect(currentDownload, &QNetworkReply::readyRead,
            this, &Downloader::downloadReadyRead);

    // prepare the file
    printf("Downloading %s...\n", url.toEncoded().constData());
    downloadTimer.start();
}

void Downloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    double speed = bytesReceived * 1000.0 / downloadTimer.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "bytes/sec";
    } else if (speed < 1024*1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024*1024;
        unit = "MB/s";
    }
}

void Downloader::downloadFinished()
{
    file.close();

    if (currentDownload->error()) {
        fprintf(stderr, "Failed: %s\n", qPrintable(currentDownload->errorString()));
        file.remove();
    } else {
        if (isHttpRedirect()) {
            reportRedirect();
            file.remove();
        } else {
            printf("Succeeded.\n");
            ++downloadedCount;
        }
    }

    currentDownload->deleteLater();
    startNextDownload();
}

void Downloader::downloadReadyRead()
{
    file.write(currentDownload->readAll());
}

bool Downloader::isHttpRedirect() const
{
    int statusCode = currentDownload->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303
           || statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void Downloader::reportRedirect()
{
    int statusCode = currentDownload->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QUrl requestUrl = currentDownload->request().url();
    QTextStream(stderr) << "Request: " << requestUrl.toDisplayString()
                        << " was redirected with code: " << statusCode
                        << '\n';

    QVariant target = currentDownload->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!target.isValid()) {
        return;
    }
    QUrl redirectUrl = target.toUrl();
    if (redirectUrl.isRelative()) {
        redirectUrl = requestUrl.resolved(redirectUrl);
    }
    QTextStream(stderr) << "Redirected to: " << redirectUrl.toDisplayString()
                        << '\n';
}