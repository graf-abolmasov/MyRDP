#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#ifndef _POSIX_VERSION
#include <windows.h>
#endif

#include "qclient.h"
#include "qtcpclientconnection.h"


QClient::QClient(int &argc, char **argv) :
    QApplication(argc, argv)
{
    myClientConnection = new QTcpClientConnection(new QTcpSocket);
    connect(myClientConnection, SIGNAL(disconnected()),
            this, SLOT(disconnected()));
    connect(myClientConnection, SIGNAL(connected()),
            this, SLOT(sayHello()));
    connect(myClientConnection, SIGNAL(error(QAbstractSocket::SocketError,QString)),
            this, SLOT(reconnect(QAbstractSocket::SocketError,QString)));
    connect(myClientConnection, SIGNAL(packetRecieved(QMap<QString,QVariant>,QByteArray)),
            this, SLOT(parsePacket(QMap<QString,QVariant>,QByteArray)));
}

QClient::~QClient()
{
    myClientConnection->disconnectFromHost();
    delete myClientConnection;
}

void QClient::disconnected()
{
    connectToServer(myAddress, myPort);
}

void QClient::connectToServer(const QHostAddress &address, qint64 port)
{
    myAddress = address;
    myPort = port;
    myClientConnection->connectToHost(address, port);
}

void QClient::reconnect(const QAbstractSocket::SocketError &error, const QString &errorString)
{
    if (error == QAbstractSocket::HostNotFoundError || error == QAbstractSocket::ConnectionRefusedError) {
#ifdef _POSIX_VERSION
        sleep(5);
#else
        Sleep(5000);
#endif
        connectToServer(myAddress, myPort);
    }
}

void QClient::sayHello()
{
    QMap<QString, QVariant> header;
    header["Hello"] = tr("Hello, Buddy!");
    myClientConnection->sendPacket(header, QByteArray());
}

void QClient::sayGoodBy()
{
    QMap<QString, QVariant> header;
    header["Good by"] = tr("I hope to see you, Buddy, again!");
    myClientConnection->sendPacket(header, QByteArray());
}

void QClient::parsePacket(const QMap<QString, QVariant> &header, const QByteArray &data)
{
    if (header.contains("FileName")) {
        // Получили файл
        QFile file(header["FileName"].toString());
        file.open(QFile::WriteOnly);
        file.write(data);
        file.close();
    }
    else if (header.contains("Command")) {
        //Другая команда
        doCommand(header, data);
    }
}

void QClient::doCommand(const QMap<QString, QVariant> &header, const QByteArray &data)
{
    if (header[tr("Command")] == tr("Quit")) {
        sayGoodBy();
        myClientConnection->disconnectFromHost();
        qApp->exit();
    } else if (header[tr("Command")] == tr("Capture screen")) {
        capScreen(header[tr("Format")].toString());
    } else if (header[tr("Command")] == tr("Dir")) {
        dir(header[tr("Path")].toString());
    } else if (header[tr("Command")] == tr("Download")) {
        download(header[tr("Path")].toString());
    } else if (header[tr("Command")] == tr("Del")) {
        del(header[tr("Path")].toString(), header[tr("Recursively")].toBool());
    } else if (header[tr("Command")] == tr("Mkdir")) {
        mkdir(header[tr("Path")].toString());
    } else if (header[tr("Command")] == tr("Rename")) {
        rename(header[tr("OldName")].toString(), header[tr("NewName")].toString());
    } else {
        QMap<QString, QVariant> responseHeader;
        responseHeader[tr("Response")] = header[tr("Command")].toString() + tr(" unknown command. Sorry :\'(. Type \"help\" for list of all command.");
    }
}

void QClient::capScreen(const QString &format)
{
    QPixmap img = QPixmap::grabWindow(QApplication::desktop()->winId());

    QMap<QString, QVariant> responseHeader;
    responseHeader[tr("Screenshot")];
    responseHeader[tr("Width")] = img.width();
    responseHeader[tr("Height")] = img.height();
    responseHeader[tr("Format")] = format;

    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, format.toAscii());
    buffer.close();

    myClientConnection->sendPacket(responseHeader, data);
}

void QClient::dir(const QString &path)
{
    QByteArray data;
    QMap<QString, QVariantList> result;

    if (path == "") {
#ifdef _POSIX_VERSION
        result[tr("/")].append(true);
        result[tr("/")].append(0);
        result[tr("/")].append(QFileInfo(tr("/")).lastModified());
#else
        foreach(QFileInfo childFinfo, QDir::drives()){
            result[childFinfo.canonicalPath()].append(childFinfo.isDir());
            result[childFinfo.canonicalPath()].append(childFinfo.isDir() ? 0 : childFinfo.size());
            result[childFinfo.canonicalPath()].append(childFinfo.lastModified());
        }
#endif
    } else {
        QDir finfo(path);
        int i = 0;
        foreach(QFileInfo childFinfo, finfo.entryInfoList(QDir::NoFilter, QDir::DirsFirst | QDir::Name)){
            if(i > 1) {
                result[childFinfo.canonicalFilePath()].append(childFinfo.isDir());
                result[childFinfo.canonicalFilePath()].append(childFinfo.isDir() ? 0 : childFinfo.size());
                result[childFinfo.canonicalFilePath()].append(childFinfo.lastModified());
            }
            i++;
        }
    }
    //сериализация
    QDataStream *stream = new QDataStream(&data, QIODevice::WriteOnly);
    (*stream) << result;
    delete stream;

    QMap<QString, QVariant> responseHeader;
    responseHeader[tr("FileTree")];
    responseHeader[tr("FileCount")] = result.count();

    myClientConnection->sendPacket(responseHeader, data);
}

void QClient::del(const QString &path, bool recursively)
{

}

void QClient::rename(const QString &oldName, const QString &newName)
{
    QDir dir;
    if (dir.exists(oldName))
        dir.rename(oldName, newName);
}

void QClient::mkdir(const QString &path)
{
    QDir dir;
    dir.mkpath(path);
}

void QClient::download(const QString &path)
{
    QMap<QString,QVariant> header;
    QFileInfo fileInfo(path);
    if (!fileInfo.exists()) {
      header[tr("FileName")] = tr("File not found!");
      myClientConnection->sendPacket(header, QByteArray());
      return;
    }

    if (fileInfo.isFile()) {
        QFile file(path);
        header[tr("FileName")] = fileInfo.fileName();
        file.open(QFile::ReadOnly);
        myClientConnection->sendPacket(header, file.readAll());
        file.close();
    }
}
