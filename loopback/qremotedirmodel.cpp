#include "qremotedirmodel.h"

#include <QStringList>

QTreeItem::QTreeItem(QTcpClientConnection *con, const QString &path, Type type, quint64 size, QDateTime modifyDate, QTreeItem *parent)
    : QObject(parent)
{
    parentItem = parent;
    myPath = path;
    myType = type;
    mySize = size;
    isPopulated = false;
    myConnection = con;
    myModifyDate = modifyDate;
}

QTreeItem::~QTreeItem()
{
    qDeleteAll(childItems);
}

QVariant QTreeItem::data(int column) const
{
    QVariant result;
    switch (column){
    case 0: result = QObject::tr("/") + myPath.split(QObject::tr("/")).last();
        break;
    case 1: result = mySize;
        break;
    case 2: result = myModifyDate;
        break;
    }
    return result;
}

int QTreeItem::row() const
{
    if (parentItem)
        return parentItem->childItems.indexOf(const_cast<QTreeItem*>(this));

    return 0;
}

void QTreeItem::populate()
{
    // Чтобы не спрашивать то, что уже получено
    if (isPopulated) {
        return;
    }

    QMap<QString,QVariant> header;
    header[tr("Command")] = tr("Dir");
    header[tr("Path")] = myPath;

    myConnection->sendPacket(header, QByteArray());

    connect(myConnection, SIGNAL(packetRecieved(QMap<QString,QVariant>,QByteArray)),
            this, SLOT(childRecieved(QMap<QString,QVariant>,QByteArray)));

}

void QTreeItem::childRecieved(QMap<QString,QVariant> header,QByteArray data)
{
    // Игнорим ответы на повторные запросы, сделанные до получения первого ответа.
    // Первый ответ считаем единственным и верным.
    if (isPopulated)
        return;

    if (header.contains(tr("FileTree"))) {
        isPopulated = true;
        //десериализиция
        QMap<QString, QVariantList> files;
        QDataStream *stream = new QDataStream(&data, QIODevice::ReadOnly);
        (*stream) >> files;
        delete stream;

        QMap<QString, QVariantList>::const_iterator i;
        for (i = files.constBegin(); i != files.constEnd(); ++i) {
            QTreeItem *newItem = appendChild(new QTreeItem(myConnection,
                                                           i.key(),
                                                           i.value().at(0).toBool() ? Directory : File,
                                                           i.value().at(1).toInt(),
                                                           i.value().at(2).toDateTime(),
                                                           this));
            connect(newItem, SIGNAL(populated(QTreeItem*)), this, SIGNAL(populated(QTreeItem*)));
        }

        disconnect(myConnection, SIGNAL(packetRecieved(QMap<QString,QVariant>,QByteArray)),
                   this, SLOT(childRecieved(QMap<QString,QVariant>,QByteArray)));

        emit populated(this);
    }
}

bool QTreeItem::isChildOf(QTreeItem *parent)
{
    return this->myPath.contains(parent->myPath);
}

/*!
  ***************************************
  * QRemoteDirModel                     *
  ***************************************/
QRemoteDirModel::QRemoteDirModel(QTcpClientConnection *con, QObject *parent) :
    QAbstractItemModel(parent)
{
    myConnection = con;
    rootItem = new QTreeItem(con, tr(""), QTreeItem::Root, -1,  QDateTime::currentDateTime());
    connect(rootItem, SIGNAL(populated(QTreeItem*)), this, SLOT(populated(QTreeItem*)));
    rootItem->populate();
}

QRemoteDirModel::~QRemoteDirModel()
{
    delete rootItem;
}

int QRemoteDirModel::columnCount(const QModelIndex &parent) const
{
    return 3;
}

QVariant QRemoteDirModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DecorationRole) {
        QTreeItem *item = static_cast<QTreeItem *>(index.internalPointer());
        if (index.column() == 0)
            return (item->isDirectory() ? myStyle.standardIcon(QStyle::SP_DirIcon) : myStyle.standardIcon(QStyle::SP_FileIcon));
    }

    if (role == Qt::DisplayRole) {
        QTreeItem *item = static_cast<QTreeItem*>(index.internalPointer());
        return item->data(index.column());
    }

    return QVariant();
}

QVariant QRemoteDirModel::headerData(int column, Qt::Orientation orientation,
                                     int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (column){
        case 0: return tr("Имя");
            break;
        case 1: return tr("Размер");
            break;
        case 2: return tr("Дата изменения");
            break;
        }
    }

    return QVariant();
}

QModelIndex QRemoteDirModel::index(int row, int column, const QModelIndex &parent) const
{

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    QTreeItem *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<QTreeItem*>(parent.internalPointer());

    QTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();

}

QModelIndex QRemoteDirModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    QTreeItem *childItem = static_cast<QTreeItem*>(index.internalPointer());
    QTreeItem *parentItem = childItem->parent();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int QRemoteDirModel::rowCount(const QModelIndex &parent) const
{
    QTreeItem *parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<QTreeItem*>(parent.internalPointer());

    return parentItem->childCount();
}

void QRemoteDirModel::populate(const QModelIndex &index)
{
    QTreeItem *item = static_cast<QTreeItem* >(index.internalPointer());
    item->populate();
}

void QRemoteDirModel::populated(QTreeItem *item)
{
    QModelIndex idx = indexOf(item, QModelIndex());
    beginInsertRows(idx, 0, item->childCount()-1);
    endInsertRows();
    emit populated(idx);
}

QModelIndex QRemoteDirModel::indexOf(QTreeItem *item, const QModelIndex &start) const
{
    int rc = rowCount(start);

    for (int i = 0 ; i < rc ; i++) {
        QModelIndex child = index(i, 0, start);

        QTreeItem* currItem = static_cast<QTreeItem*>(child.internalPointer());

        if (currItem == item) {
            return child;
        } else {
            if (item->isChildOf(currItem))
                return indexOf(item, child);
        }
    }

    return QModelIndex();
}

void QRemoteDirModel::download(const QModelIndex &index)
{
    if (index.isValid()) {
        QMap<QString,QVariant> header;
        header[tr("Command")] = tr("Download");
        header[tr("Path")] = static_cast<QTreeItem *>(index.internalPointer())->path();
        myConnection->sendPacket(header, QByteArray());
    }
}

void QRemoteDirModel::upload(const QModelIndex &index, const QString &name)
{
    if (index.isValid()) {
        QMap<QString,QVariant> header;
        QTreeItem *item = static_cast<QTreeItem *>(index.internalPointer());
        QFileInfo fileInfo(name);
        if (fileInfo.isFile() && item->isDirectory()){
            QFile file(name);
            header[tr("FileName")] = item->path() + tr("/") + fileInfo.fileName();
            file.open(QFile::ReadOnly);
            myConnection->sendPacket(header, file.readAll());
            file.close();
        }
    }
}

void QRemoteDirModel::mkDir(const QModelIndex &index, const QString &name)
{
    if (index.isValid()) {
        QMap<QString,QVariant> header;
        QTreeItem *item = static_cast<QTreeItem *>(index.internalPointer());
        if (item->isDirectory()){
            header[tr("Command")] = tr("Mkdir");
            header[tr("Path")] = item->path() + tr("/") + name;
            myConnection->sendPacket(header, QByteArray());
        }
    }
}

void QRemoteDirModel::del(const QModelIndex &index, bool recursively)
{
    if (index.isValid()) {
        QMap<QString,QVariant> header;
        QTreeItem *item = static_cast<QTreeItem *>(index.internalPointer());
        header[tr("Command")] = tr("Del");
        header[tr("Path")] = item->path();
        header[tr("Recursively")] = recursively;
        myConnection->sendPacket(header, QByteArray());
    }
}

void QRemoteDirModel::rename(const QModelIndex &index, const QString &newName)
{
    if (index.isValid()) {
        QMap<QString,QVariant> header;
        QTreeItem *item = static_cast<QTreeItem *>(index.internalPointer());
        header[tr("Command")] = tr("Rename");
        header[tr("OldName")] = item->path();
        header[tr("NewName")] = item->parent()->path() + tr("/") + newName;
        myConnection->sendPacket(header, QByteArray());
    }
}
