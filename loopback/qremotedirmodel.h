#ifndef QREMOTEDIRMODEL_H
#define QREMOTEDIRMODEL_H

#include <QtCore>
#include <QtGui>
#include "qtcpclientconnection.h"

class QTreeItem : public QObject
{
    Q_OBJECT

public:
    enum Type {Root, File, Directory};
    QTreeItem(QTcpClientConnection *con, const QString &path, Type type, quint64 size, QDateTime modifyDate, QTreeItem *parent = 0);
    ~QTreeItem();

    QTreeItem *appendChild(QTreeItem *child) { childItems.append(child);  return child; }
    QTreeItem *child(int row) { return childItems.value(row); }
    int childCount() const { return childItems.count(); }
    QTreeItem *parent() { return parentItem; }
    QString path() { return myPath; }
    bool isDirectory() { return (myType == Directory); }
    bool isChildOf(QTreeItem *parent);

    QVariant data(int column) const;
    int row() const;
    void populate();
private slots:
    void childRecieved(QMap<QString,QVariant>,QByteArray);

signals:
    void populated(QTreeItem *item);

private:
    QList<QTreeItem*> childItems;
    quint64 mySize;
    QString myPath;
    QTreeItem *parentItem;
    bool isPopulated;
    Type myType;
    QDateTime myModifyDate;
    QTcpClientConnection *myConnection;
};

class QRemoteDirModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    QRemoteDirModel(QTcpClientConnection * con, QObject *parent = 0);
    ~QRemoteDirModel();

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex indexOf(QTreeItem *item, const QModelIndex& start) const;

    void download(const QModelIndex &index);
    void upload(const QModelIndex &index, const QString &name);
    void mkDir(const QModelIndex &index, const QString &name);
    void del(const QModelIndex &index, bool recursively);
    void rename(const QModelIndex &index, const QString &newName);

public slots:
    void populate(const QModelIndex &index);
    void populated(QTreeItem *item);

signals:
    void populated(const QModelIndex &index);

private:
    QTreeItem *rootItem;
    QTcpClientConnection *myConnection;
    QCommonStyle myStyle;



};

#endif // QREMOTEDIRMODEL_H
