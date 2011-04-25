#ifndef QTCPCLIENTCONNECTION_H
#define QTCPCLIENTCONNECTION_H

#include <QtCore>
//#include <QtGui>
#include <QtNetwork>
#include <QTcpSocket>

#define HEADER_SIZE 64*1024
#define COMPRESS_LEVEL 6

static qint64 nextClientId = 0;

class QTcpClientConnection : public QObject
{
    Q_OBJECT
public:

    qint64 clientId;

    QTcpClientConnection(QTcpSocket *socket, QObject *parent = 0);
    QHostAddress remoteAddress();
    void connectToHost(const QHostAddress &address, qint16 port);
    void disconnectFromHost();
    void sendPacket(QMap<QString, QVariant> header, const QByteArray &data);
    QString errorString();

signals:
    void headerRecieved(const QMap<QString, QVariant> &header);
    void packetRecieved(const QMap<QString, QVariant> &header, const QByteArray &data);
    void recieveProgress(const qint64 &numBytes, const qint64 &bytesTotal);
    void error(const QAbstractSocket::SocketError &error, const QString &errorString);
    void packetSent();
    void sendProgress(const qint64 &numBytes, const qint64 &bytesTotal);
    void connected();
    void disconnected();

public slots:
    void readSlot();
    void writeSlot(qint64 numBytes);

private slots:
    void extendError(const QAbstractSocket::SocketError &error);

private:
    QTcpSocket* myTcpSocket;
    QString lastError;

    //Для приема
    qint64 bytesRecieved;
    QByteArray recievedBuffer;
    QByteArray recvData;
    void parseHeader(QByteArray rawHeader);
    QMap<QString, QVariant> header;

    //Для отправки
    qint64 bytesTotal;
    qint64 bytesWritten;
};

#endif // QTCPCLIENTCONNECTION_H
