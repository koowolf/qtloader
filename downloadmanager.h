#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H

#include <QCoreApplication>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>
#include <QStringList>
#include <QTimer>
#include <QUrl>

#include <stdio.h>

class QSslError;
class MainWindow;

QT_USE_NAMESPACE

class DownloadManager: public QObject
{
    Q_OBJECT
    QNetworkAccessManager manager;

public:
    DownloadManager(MainWindow* pDelegate);
    void doDownload(const QUrl &url);
    void startDownload(QString strUrl);

public slots:
    void httpReadyRead();
    void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);
    void downloadFinished();
    void sslErrors(const QList<QSslError> &errors);
    void slotAuthenticationRequired(QNetworkReply*,QAuthenticator *);

private:
    MainWindow* m_pDelegate;
    QNetworkReply *reply;
    QUrl url;
    QFile *file;
    int httpGetId;
    bool httpRequestAborted;
};

#endif // DOWNLOADMANAGER_H
