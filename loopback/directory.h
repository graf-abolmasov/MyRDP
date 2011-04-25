#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QtGui>
#include <QtCore>
#include "qremotedirmodel.h"
#include "qtcpclientconnection.h"

namespace Ui {
    class Directory;
}

class Directory : public QWidget
{
    Q_OBJECT

public:
    Directory(QTcpClientConnection *con, QWidget *parent = 0);
    ~Directory();

private slots:
    void on_downloadButton_clicked();
    void refreshView(const QModelIndex &index);
    void on_treeView_clicked(QModelIndex index);

    void on_uploadButton_clicked();

private:
    Ui::Directory *ui;
    QRemoteDirModel *model;
    QTcpClientConnection *myConnection;

    bool ignorePacket;
};

#endif // DIRECTORY_H
