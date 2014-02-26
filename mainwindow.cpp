#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDesktopWidget>
#include <QProcess>
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include "unzip.h"
using namespace std;

#define BUFFER_SIZE    8192
#define MAX_FILENAME   512

bool MainWindow::createDirectory(const char *path)
{
    QDir dir;
    bool bRet = false;
    if (dir.exists(path) == false)
        bRet = dir.mkpath(path);
    else
        return true;
    return bRet;
}

bool MainWindow::unCompress(string strZipPath, string strDstFolder)
{
    // Open the zip file
    unzFile zipfile = unzOpen(strZipPath.c_str());
    if (! zipfile)
    {
        refreshStatus(tr("can not open downloaded zip file %1").arg(strZipPath.c_str()));
        return false;
    }

    // Get info about the zip file
    unz_global_info global_info;
    if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK)
    {
        refreshStatus(tr("can not read file global info of %1").arg(strZipPath.c_str()));
        unzClose(zipfile);
        return false;
    }

    // Buffer to hold data read from the zip file
    char readBuffer[BUFFER_SIZE];

    refreshStatus(tr("start uncompressing"));

    // Loop to extract all files.
    uLong i;
    for (i = 0; i < global_info.number_entry; ++i)
    {
        // Get info about current file.
        unz_file_info fileInfo;
        char fileName[MAX_FILENAME];
        if (unzGetCurrentFileInfo(zipfile,
                                  &fileInfo,
                                  fileName,
                                  MAX_FILENAME,
                                  NULL,
                                  0,
                                  NULL,
                                  0) != UNZ_OK)
        {
            refreshStatus(tr("can not read file info"));
            unzClose(zipfile);
            return false;
        }

        const string fullPath = strDstFolder + (strDstFolder[strDstFolder.length() - 1] == '/' ? "" : "/") + fileName;

        // Check if this entry is a directory or a file.
        const size_t filenameLength = strlen(fileName);
        if (fileName[filenameLength-1] == '/')
        {
            // Entry is a direcotry, so create it.
            // If the directory exists, it will failed scilently.
            if (!createDirectory(fullPath.c_str()))
            {
                refreshStatus(tr("can not create directory %1").arg(fullPath.c_str()));
                unzClose(zipfile);
                return false;
            }
        }
        else
        {
            //There are not directory entry in some case.
            //So we need to test whether the file directory exists when uncompressing file entry
            //, if does not exist then create directory
            const string fileNameStr(fileName);

            size_t startIndex=0;

            size_t index=fileNameStr.find("/",startIndex);

            while(index != std::string::npos)
            {
                const string dir = strDstFolder + (strDstFolder[strDstFolder.length() - 1] == '/' ? "" : "/") + fileNameStr.substr(0,index);

                FILE *out = fopen(dir.c_str(), "r");

                if(!out)
                {
                    if (!createDirectory(dir.c_str()))
                    {
                        refreshStatus(tr("can not create directory %1").arg(fullPath.c_str()));
                        unzClose(zipfile);
                        return false;
                    }
                    else
                    {
                        refreshStatus(tr("create directory %1").arg(dir.c_str()));
                    }
                }
                else
                {
                    fclose(out);
                }

                startIndex=index+1;

                index=fileNameStr.find("/",startIndex);

            }



            // Entry is a file, so extract it.

            // Open current file.
            if (unzOpenCurrentFile(zipfile) != UNZ_OK)
            {
                refreshStatus(tr("can not open file %1").arg(fileName));
                unzClose(zipfile);
                return false;
            }

            // Create a file to store current file.
            FILE *out = fopen(fullPath.c_str(), "wb");
            if (! out)
            {
                refreshStatus(tr("can not open destination file %1").arg(fullPath.c_str()));
                unzCloseCurrentFile(zipfile);
                unzClose(zipfile);
                return false;
            }

            // Write current file content to destinate file.
            int error = UNZ_OK;
            do
            {
                error = unzReadCurrentFile(zipfile, readBuffer, BUFFER_SIZE);
                if (error < 0)
                {
                    refreshStatus(tr("can not read zip file %1, error code is %2").arg(fileName).arg(error));
                    unzCloseCurrentFile(zipfile);
                    unzClose(zipfile);
                    return false;
                }

                if (error > 0)
                {
                    fwrite(readBuffer, error, 1, out);
                }
            } while(error > 0);

            fclose(out);
        }

        unzCloseCurrentFile(zipfile);

        // Goto next entry listed in the zip file.
        if ((i+1) < global_info.number_entry)
        {
            if (unzGoToNextFile(zipfile) != UNZ_OK)
            {
                refreshStatus(tr("can not read next file"));
                unzClose(zipfile);
                return false;
            }
        }
    }

    refreshStatus(tr("end uncompressing: %1").arg(strDstFolder.c_str()));
    unzClose(zipfile);

    return true;
}

void MainWindow::resizeEvent ( QResizeEvent * event )
{
    QWidget::resizeEvent(event);
    QPalette pal(palette());
    QPixmap bgImage(":/myresources/bg.png");

    pal.setBrush(QPalette::Window,QBrush(bgImage.scaled(event->size(), Qt::IgnoreAspectRatio,Qt::SmoothTransformation)));
    setPalette(pal);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowFlags(this->windowFlags() & ~Qt::WindowMaximizeButtonHint);
    this->setFixedSize(this->size().width(), this->size().height());

    m_sSettingsFile = QApplication::applicationDirPath() + "/download.ini";

    setProperty("base", "download_url", "https://raw.github.com/koowolf/cjc-test/master/mygtest/folder.zip");
    setProperty("base", "destination_file", "F://download.zip");
    setProperty("base", "uncompress_destination_folder", "C://folder");
    setProperty("variables", "EXAMPLE", "EXAMPLE_VALUE");

    m_url = getProperty("base", "download_url", true);
    ui->downbtn->setEnabled(m_url != "");

    hideProgressBar(true);

    ui->menuBar->hide();
    ui->mainToolBar->hide();
    ui->statusBar->hide();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::hideProgressBar(bool bHide)
{
    if (bHide)
        ui->progressBar->hide();
    else
        ui->progressBar->show();
}

void MainWindow::enableDownloadBtn()
{
    ui->downbtn->setEnabled(true);
}

void MainWindow::refreshStatus(QString strText)
{
    ui->statusLabel->setText(strText);
}

QString MainWindow::getDestinationFile()
{
    return getProperty("base", "destination_file");
}


void MainWindow::setEnvironmentVariables()
{
#ifdef PLATFORM_WIN32
    refreshStatus("write user variables...");

    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    settings.beginGroup("variables");
    foreach (QString key, settings.allKeys())
    {
        QString strValue = settings.value(key, "").toString();
        QSettings reg_settings("HKEY_CURRENT_USER\\Environment", QSettings::NativeFormat);
        reg_settings.setValue(key, strValue);
    }
    settings.endGroup();

    refreshStatus("write user variables done...");
#endif
}


void MainWindow::extractZipFile(QString strFilePath)
{
    unCompress(strFilePath.toStdString(), getProperty("base", "uncompress_destination_folder").toStdString());

    setEnvironmentVariables();
}

void MainWindow::updateDataReadProgress(qint64 bytesRead, qint64 totalBytes)
{
    ui->progressBar->setMaximum(totalBytes);
    ui->progressBar->setValue(bytesRead);

    double now = (double)bytesRead / 1048576.0;
    double left = (double)totalBytes / 1048576.0;
    QString strText(tr(" %1 /%2 M").arg(now, 4).arg(left, 4));
    ui->progressLabel->setText(strText);
}

void MainWindow::on_downbtn_clicked()
{
    this->ui->downbtn->setEnabled(false);
    this->ui->statusLabel->setText("downloading...");
    hideProgressBar(false);

    m_pMmanager = new DownloadManager(this);
    m_pMmanager->startDownload(m_url);
}

void MainWindow::setProperty(QString group, QString key, QString value)
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    settings.beginGroup(group);
    settings.setValue(key, value);
    settings.endGroup();
}

QString MainWindow::getProperty(QString group, QString key, bool bShowWarning /*= false*/)
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    settings.beginGroup(group);
    QString strValue = settings.value(key, "").toString();
    settings.endGroup();
    if (strValue == "" && bShowWarning)
        QMessageBox::warning(this, "error", tr("the value of key[%1] in download.ini is empty, please check.").arg(key));
    return strValue;
}


