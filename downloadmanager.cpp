#include "downloadmanager.h"
#include <QMessageBox>
#include "mainwindow.h"
#include <QDir>
#include "ui_authenticationdialog.h"
#include <QAuthenticator>


DownloadManager::DownloadManager(MainWindow* pDelegate):
    m_pDelegate(pDelegate)
{
}

void DownloadManager::doDownload(const QUrl &url)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/zip");

    reply = manager.get(request);

    // mainwindow
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
           (QObject *)m_pDelegate, SLOT(updateDataReadProgress(qint64,qint64)));

    connect(reply, SIGNAL(finished()),
            this, SLOT(downloadFinished()));
    connect(reply, SIGNAL(readyRead()),
            this, SLOT(httpReadyRead()));
    connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
            this, SLOT(updateDataReadProgress(qint64,qint64)));
    connect(&manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)),
            this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
#ifndef QT_NO_SSL
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), SLOT(sslErrors(QList<QSslError>)));
#endif
}

void DownloadManager::httpReadyRead()
{
    // this slot gets called every time the QNetworkReply has new data.
    // We read all of its new data and write it into the file.
    // That way we use less RAM than when reading it at the finished()
    // signal of the QNetworkReply
    if (file)
        file->write(reply->readAll());
}

void DownloadManager::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    if (httpRequestAborted)
        return;
}

void DownloadManager::startDownload(QString strUrl)
{
    url = QUrl::fromEncoded(strUrl.toLocal8Bit());

    QString fileName = m_pDelegate->getDestinationFile();

    if (fileName.isEmpty()){
        QFileInfo fileInfo(url.path());
        fileName = fileInfo.fileName();
        if (fileName.isEmpty())
            fileName = "index.html";
    }

    if (QFile::exists(fileName)) {        
        if (QMessageBox::question((QWidget *)m_pDelegate, tr("HTTP"), tr("There already exists a file called %1 in "
                                                                         "the current directory. Overwrite?").arg(fileName), QMessageBox::Yes|QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        {
            m_pDelegate->refreshStatus("cancel download.");
            m_pDelegate->hideProgressBar(true);
            m_pDelegate->enableDownloadBtn();
            return;
        }
        QFile::remove(fileName);
    }

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly)) {
        QMessageBox::information((QWidget *)m_pDelegate, tr("HTTP"),
                                 tr("Unable to save the file %1: %2.")
                                 .arg(fileName).arg(file->errorString()));
        delete file;
        file = 0;
        return;
    }

    m_pDelegate->refreshStatus(tr("Downloading %1.").arg(fileName));

    // schedule the request
    httpRequestAborted = false;
    doDownload(url);
}

void DownloadManager::sslErrors(const QList<QSslError> &sslErrors)
{
#ifndef QT_NO_SSL
    foreach (const QSslError &error, sslErrors)
        fprintf(stderr, "SSL error: %s\n", qPrintable(error.errorString()));
#else
    Q_UNUSED(sslErrors);
#endif
}

void DownloadManager::downloadFinished()
{
    if (httpRequestAborted) {
        if (file) {
            file->close();
            file->remove();
            delete file;
            file = 0;
        }
        reply->deleteLater();

        m_pDelegate->hideProgressBar(true);
        return;
    }

    m_pDelegate->hideProgressBar(true);
    file->flush();
    file->close();

    QVariant redirectionTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (reply->error()) {
        file->remove();
        QMessageBox::information(NULL, tr("HTTP"),
                                 tr("Download failed: %1.")
                                 .arg(reply->errorString()));

        m_pDelegate->refreshStatus(tr("Download failed: %1.")
                                   .arg(reply->errorString()));
        m_pDelegate->enableDownloadBtn();
    } else if (!redirectionTarget.isNull()) {
        QUrl newUrl = url.resolved(redirectionTarget.toUrl());
        if (QMessageBox::question(NULL, tr("HTTP"),
                                  tr("Redirect to %1 ?").arg(newUrl.toString()),
                                  QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            url = newUrl;
            reply->deleteLater();
            file->open(QIODevice::WriteOnly);
            file->resize(0);
            startDownload(url.toString());
            return;
        }
    } else {

        QString fileName = file->fileName();
        m_pDelegate->refreshStatus(tr("%1 Downloaded done!").arg(fileName));
        m_pDelegate->enableDownloadBtn();

        m_pDelegate->extractZipFile(m_pDelegate->getDestinationFile());
    }


    reply->deleteLater();
    reply = 0;
    delete file;
    file = 0;
}

void DownloadManager::slotAuthenticationRequired(QNetworkReply*,QAuthenticator *authenticator)
{
    QDialog dlg;
    Ui::Dialog ui;
    ui.setupUi(&dlg);
    dlg.adjustSize();
    ui.siteDescription->setText(tr("%1 at %2").arg(authenticator->realm()).arg(url.host()));

    // Did the URL have information? Fill the UI
    // This is only relevant if the URL-supplied credentials were wrong
    ui.userEdit->setText(url.userName());
    ui.passwordEdit->setText(url.password());

    if (dlg.exec() == QDialog::Accepted) {
        authenticator->setUser(ui.userEdit->text());
        authenticator->setPassword(ui.passwordEdit->text());
    }
}
