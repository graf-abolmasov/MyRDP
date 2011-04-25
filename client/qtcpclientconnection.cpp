#include "qtcpclientconnection.h"

QTcpClientConnection::QTcpClientConnection(QTcpSocket *socket, QObject *parent) :
    QObject(parent)
{
    myTcpSocket = socket;
    connect(socket, SIGNAL(readyRead()),
            this, SLOT(readSlot()));
    connect(socket, SIGNAL(disconnected()),
            this, SIGNAL(disconnected()));
    connect(socket, SIGNAL(connected()),
            this, SIGNAL(connected()));
    connect(socket, SIGNAL(bytesWritten(qint64)),
            this, SLOT(writeSlot(qint64)));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(extendError(QAbstractSocket::SocketError)));
    bytesRecieved = 0;
    clientId = nextClientId++;
}

void QTcpClientConnection::readSlot()
{
    // Ждем заголовок, если получено данных больше чем заголовок,
    // то в них точно есть заголовок
    if (bytesRecieved < HEADER_SIZE) {
        int bytesToRead = HEADER_SIZE - bytesRecieved;
        bytesRecieved += qMin((int)myTcpSocket->bytesAvailable(), bytesToRead);
        recievedBuffer.append(myTcpSocket->read(bytesToRead));
    }

    if (bytesRecieved == HEADER_SIZE && header.isEmpty()) {
        QByteArray rawHeader(recievedBuffer.mid(0, HEADER_SIZE));
        parseHeader(rawHeader);
        recievedBuffer.clear();
        emit headerRecieved(header);
    }

    // Заголовок пришел. Сейчас пойдут только данные
    if (header.value(tr("DataSize")).toInt() > 0 && bytesRecieved < (HEADER_SIZE + header.value(tr("DataSize")).toInt())) {
        // Данные должны быть
        int bytesToRead = header[tr("DataSize")].toInt() - bytesRecieved + HEADER_SIZE;
        bytesRecieved += qMin((int)myTcpSocket->bytesAvailable(), bytesToRead);
        recievedBuffer.append(myTcpSocket->read(bytesToRead));
        emit recieveProgress(bytesRecieved - HEADER_SIZE, header[tr("DataSize")].toInt());
        recvData.append(recievedBuffer);
        recievedBuffer.clear();
    }

    // Пришли все данные?
    if (bytesRecieved == (HEADER_SIZE + header.value(tr("DataSize")).toInt())) {
        QByteArray uncompressedData = qUncompress(recvData);
        emit packetRecieved(header, uncompressedData);
        header.clear();
        recievedBuffer.clear();
        recvData.clear();
        bytesRecieved = 0;
    }
}

void QTcpClientConnection::parseHeader(QByteArray rawHeader)
{
    //десериализиция
    QDataStream *stream = new QDataStream(&rawHeader, QIODevice::ReadOnly);
    (*stream) >> header;
    delete stream;
}

QHostAddress QTcpClientConnection::remoteAddress()
{
    return myTcpSocket->peerAddress();
}

void QTcpClientConnection::sendPacket(QMap<QString, QVariant> header, const QByteArray &data)
{
    QByteArray sendBuffer;
    QByteArray compressedData = qCompress(data, COMPRESS_LEVEL);

    header[tr("DataSize")] = compressedData.size();

    //сериализация
    QDataStream *stream = new QDataStream(&sendBuffer, QIODevice::WriteOnly);
    (*stream) << header;
    delete stream;

    sendBuffer.append(QByteArray(HEADER_SIZE - sendBuffer.size(), 0));
    sendBuffer.append(compressedData);
    bytesWritten = 0;
    bytesTotal = sendBuffer.size();
    myTcpSocket->write(sendBuffer);
}

void QTcpClientConnection::connectToHost(const QHostAddress &address, qint16 port)
{
    myTcpSocket->connectToHost(address, port);
}

void QTcpClientConnection::disconnectFromHost()
{
    myTcpSocket->disconnectFromHost();
}

void QTcpClientConnection::writeSlot(qint64 numBytes)
{
    this->bytesWritten += numBytes;
    emit sendProgress(bytesWritten, bytesTotal);
    if (bytesWritten == bytesTotal) {
        emit packetSent();
    }
}

QString QTcpClientConnection::errorString()
{
    return lastError;
}

void QTcpClientConnection::extendError(const QAbstractSocket::SocketError &e)
{
    switch (e) {
    case QAbstractSocket::ConnectionRefusedError: lastError = tr("Сервер отклонил соединение.");
        break;
    case QAbstractSocket::SocketTimeoutError: lastError = tr("Превышено время ожидания.");
        break;
    case QAbstractSocket::RemoteHostClosedError: lastError = tr("Соединение закрыто.");
        break;
    default:
        lastError = tr("");
    }

    emit error(e, errorString());
}

