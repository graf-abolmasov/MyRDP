#include <QtGui>
#include <QtNetwork>
#include "qclientmanager.h"

/*! *************************************
    * class TableItem                   *
    *************************************/
TableItem::TableItem(QTcpSocket *socket, QObject *parent) :
    QTcpClientConnection(socket, parent)
{
}

QString TableItem::info()
{
    QString result = tr("Клиент ID = ");
    result.append(QString::number(clientId));
    return result;
}

/*! *************************************
    * class QProgressDelegate           *
    *************************************/
QProgressDelegate::QProgressDelegate(QObject *parent) :
    QStyledItemDelegate(parent)
{
}

void QProgressDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
    if (index.column() == 2) {
        int progress = index.data().toInt();

        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = option.rect;
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = progress;
        progressBarOption.text = QString::number(progress) + "%";
        progressBarOption.textVisible = true;

        QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                           &progressBarOption, painter);
    } else
        QStyledItemDelegate::paint(painter, option, index);
}

/*! *************************************
    * class QClientTableModel           *
    *************************************/
QClientTableModel::QClientTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{
    connect(&tcpServer, SIGNAL(newConnection()),
            this, SLOT(acceptConnection()));
}

QClientTableModel::~QClientTableModel()
{
    list.clear();
}

int QClientTableModel::rowCount(const QModelIndex &) const
{
    return list.size();
}

int QClientTableModel::columnCount(const QModelIndex &) const
{
    return 4;
}

QVariant QClientTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row()>=list.size())
        return QVariant();
//    if (role==Qt::DecorationRole)
//    {
//        if (index.column()==2)
//            return *statusToIcon(list.at(index.row()).status);
//    }
    if (role!=Qt::DisplayRole)
        return QVariant();

    switch (index.column())
    {
        case 0:
            return list[index.row()]->remoteAddress().toString();
            break;
        case 1:
            return list[index.row()]->info();
            break;
        case 2:
            return list[index.row()]->transferProgress;
            break;
        case 3:
            return list[index.row()]->errorString();
    }
    return QVariant();
}

QVariant QClientTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role!=Qt::DisplayRole)
        return QVariant();
    if (orientation!=Qt::Horizontal)
        return QVariant();
    switch (section)
    {
    case 0:
        return tr("Адрес");
        break;
    case 1:
        return tr("Инфо");
        break;
    case 2:
        return tr("Прогресс");
        break;
    case 3:
        return tr("Ошибка");
        break;
    }
    return QVariant();
}

void QClientTableModel::acceptConnection()
{
    QTcpSocket *tcpServerConnection = tcpServer.nextPendingConnection();
    beginInsertRows(QModelIndex(), list.size(), list.size());
    // Создаем нового клиента
    TableItem* item = new TableItem(tcpServerConnection);
    item->transferProgress = 0;
    connect(item, SIGNAL(recieveProgress(qint64,qint64)),
            this, SLOT(updateServerProgress(qint64,qint64)));
    connect(item , SIGNAL(packetRecieved(QMap<QString,QVariant>,QByteArray)),
            this, SLOT(parsePacket(QMap<QString,QVariant>,QByteArray)));
    connect(item , SIGNAL(disconnected()),
            this, SLOT(closeConnection()));
    connect(item , SIGNAL(error(QAbstractSocket::SocketError,QString)),
            this, SLOT(displayError(QAbstractSocket::SocketError,QString)));

    //Добавляем клиента в список
    list << item;
    endInsertRows();

    emit clientConnected(item->remoteAddress().toString());
}

void QClientTableModel::closeConnection()
{
    TableItem *con = qobject_cast<TableItem *>(sender());
    int idx = list.indexOf(con);
    if (idx != -1) {
        beginRemoveRows(QModelIndex(), idx, idx);
        list.removeAt(idx);
        endRemoveRows();
        delete con;
        emit clientDisconnected(con->clientId);
    }
}

void QClientTableModel::updateServerProgress(const qint64 &numBytes, const qint64 &totalBytes)
{
    TableItem *con = qobject_cast<TableItem *>(sender());
    int idx = list.indexOf(con);
    if (idx != -1) {
        list[idx]->transferProgress = numBytes / totalBytes * 100;
        emit dataChanged(this->index(idx, 2), this->index(idx, 2));
    }
}

void QClientTableModel::parsePacket(const QMap<QString, QVariant> &header, const QByteArray &data)
{
    QObject *s = sender();
    TableItem *con = qobject_cast<TableItem *>(s);
    int idx = list.indexOf(con);
    if (idx != -1) {
        if (header.contains(tr("FileName"))) {
            // Получили файл
            if (header["FileName"].toString() != tr("File not found!")) {
                QFile file(header["FileName"].toString());
                file.open(QFile::WriteOnly);
                file.write(data);
                file.close();
                emit fileRecieved(con->clientId, header[tr("FileName")].toString());
            }
        } else if (header.contains(tr("Command"))) {
            //Другая команда
            emit commandRecieved(con->clientId, header, data);
        }
    }
}

void QClientTableModel::displayError(const QAbstractSocket::SocketError &socketError, const QString &errorString)
{
    TableItem *con =qobject_cast<TableItem *>(sender());
    int idx = list.indexOf(con);
    if (idx != -1) {
        emit dataChanged(this->index(idx, 2), this->index(idx, 2));
    }
}

bool QClientTableModel::startListen(const QHostAddress &address, qint16 port)
{
    tcpServer.close();
    return tcpServer.listen(address, port);
}

void QClientTableModel::stopListen()
{
    tcpServer.close();
    if (list.size() > 0) {
        beginRemoveRows(QModelIndex(), 0, list.size()-1);
        list.clear();
        endRemoveRows();
    }
}
