#ifndef QCLIENTMANAG ER_H
#define QCLIENTMANAGER_H

#include <QtCore>
#include <QtGui>
#include <QtNetwork>
#include "qtcpclientconnection.h"

class TableItem : public QTcpClientConnection
{
   Q_OBJECT

public:
    TableItem(QTcpSocket *socket, QObject *parent = 0);
    QString info();
    qint64 transferProgress;
};

class QProgressDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    QProgressDelegate(QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

class QClientTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    QClientTableModel(QObject *parent = 0);
    ~QClientTableModel();

    int rowCount(const QModelIndex &) const;
    int columnCount(const QModelIndex &) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool startListen(const QHostAddress &address, qint16 port);
    void stopListen();

    QList<TableItem *> list;
signals:
    void clientConnected(const QString &info);
    void clientDisconnected(qint64 clientId);
    void fileRecieved(quint64 clientId, const QString &fileName);
    void commandRecieved(quint64 clientId, const QMap<QString, QVariant> &header, const QByteArray &data);

private slots:
    void acceptConnection();
    void closeConnection();
    void updateServerProgress(const qint64 &numBytes, const qint64 &totalBytes);
    void parsePacket(const QMap<QString, QVariant> &header, const QByteArray &data);
    void displayError(const QAbstractSocket::SocketError &socketError, const QString &errorString);

private:
    QTcpServer tcpServer;

};

#endif // QCLIENTMANAGER_H
