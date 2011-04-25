#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    myClientManager = new QClientTableModel(this);
    ui->clientsTableView->setModel(myClientManager);
    QProgressDelegate *delegate = new QProgressDelegate(myClientManager);
    ui->clientsTableView->setItemDelegate(delegate);
    ui->stopListenButton->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_listenButton_clicked()
{
    myClientManager->startListen(QHostAddress::Any, 58988);
    ui->listenButton->setEnabled(false);
    ui->stopListenButton->setEnabled(true);
}

void MainWindow::on_stopListenButton_clicked()
{
    myClientManager->stopListen();
    ui->listenButton->setEnabled(true);
    ui->stopListenButton->setEnabled(false);
}

void MainWindow::closeEvent() const
{
    myClientManager->stopListen();
}

void MainWindow::on_screenButton_clicked()
{
    int idx = ui->clientsTableView->currentIndex().row();
    QTcpClientConnection *con = myClientManager->list.at(idx);
    Screenshot *w = new Screenshot(con);
    w->show();
}

void MainWindow::on_directoryTreeButton_clicked()
{
    int idx = ui->clientsTableView->currentIndex().row();
    QTcpClientConnection *con = myClientManager->list.at(idx);
    Directory *w = new Directory(con);
    w->show();
}
