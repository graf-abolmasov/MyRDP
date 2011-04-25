#include "directory.h"
#include "ui_directory.h"

Directory::Directory(QTcpClientConnection *con, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Directory)
{
    ui->setupUi(this);
    myConnection = con;
    model = new QRemoteDirModel(con);
    ui->treeView->setModel(model);
    ui->treeView->header()->setResizeMode(QHeaderView::ResizeToContents);
    connect(ui->treeView, SIGNAL(doubleClicked(QModelIndex)),
            model, SLOT(populate(QModelIndex)));
    connect(model, SIGNAL(populated(QModelIndex)),
            this, SLOT(refreshView(QModelIndex)));
}

Directory::~Directory()
{
    delete model;
    delete ui;
}

void Directory::on_downloadButton_clicked()
{
    model->download(ui->treeView->currentIndex());
}

void Directory::refreshView(const QModelIndex &index)
{
    QTreeItem *item = static_cast<QTreeItem *>(index.internalPointer());
    ui->treeView->setExpanded(index, true);
}

void Directory::on_treeView_clicked(QModelIndex index)
{
    QTreeItem *item = static_cast<QTreeItem *>(index.internalPointer());
    ui->pathEdit->setText(item->path());
}

void Directory::on_uploadButton_clicked()
{
    QFileDialog dlg;
    dlg.setFileMode(QFileDialog::ExistingFile);
    if (dlg.exec()) {
        model->upload(ui->treeView->currentIndex(), dlg.selectedFiles().first());
    }
}
