#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QResizeEvent>
#include "downloadmanager.h"
#include <QSettings>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    virtual void 	resizeEvent ( QResizeEvent * event );

    void hideProgressBar(bool bHide);
    void enableDownloadBtn();
    void refreshStatus(QString strText);

    QString getDestinationFile();

    void extractZipFile(QString strFilePath);

public slots:
    void updateDataReadProgress(qint64 bytesRead, qint64 totalBytes);

private slots:
    void on_downbtn_clicked();

private:
    void setProperty(QString group, QString key, QString value);
    QString getProperty(QString group, QString key, bool bShowWarning = false);

    bool createDirectory(const char *path);
    bool unCompress(std::string strZipPath, std::string strDstFolder);

    void setEnvironmentVariables();

private:
    Ui::MainWindow *ui;
    DownloadManager *m_pMmanager;
    QString m_sSettingsFile;
    QString m_url;
};

#endif // MAINWINDOW_H
