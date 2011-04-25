#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui>
#include <QtCore>
#include "qclientmanager.h"
#include "screenshot.h"
#include "directory.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    //void updateServerProgress(int clientNumber, const qint64 &numBytes, const qint64 &totalBytes);
    //void displayError(QAbstractSocket::SocketError socketError);
    //void parseCommand(int clientNumber, const QMap<QString, QString> &header, const QByteArray &data);
    //void addNewClient(const QString &info);
    //void removeClient(int clientNumber);
protected:
    void closeEvent() const;

private slots:
    void on_listenButton_clicked();

    void on_stopListenButton_clicked();

    void on_screenButton_clicked();

    void on_directoryTreeButton_clicked();

private:
    QFile file;
    QClientTableModel *myClientManager;

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
