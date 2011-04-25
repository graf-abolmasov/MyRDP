#ifndef QCLIENT_H
#define QCLIENT_H

#include <QApplication>
#include <QtNetwork/QHostAddress>

class QTcpClientConnection;

class QClient : public QApplication
{
    Q_OBJECT
public:
    QClient(int &argc, char **argv);
    ~QClient();
    void connectToServer(const QHostAddress &address, qint64 port);
private slots:
    void disconnected();
    void reconnect(const QAbstractSocket::SocketError &error,  const QString &errorString);
    void parsePacket(const QMap<QString, QVariant> &header, const QByteArray &data);
    void doCommand(const QMap<QString, QVariant> &header, const QByteArray &data);
    void sayHello();
    void sayGoodBy();
private:
    //команды
    void capScreen(const QString &format);
    void dir(const QString &path);
    void download(const QString &path);
    void del(const QString &path, bool recursively);
    void mkdir(const QString &path);
    void rename(const QString &oldName, const QString &newName);

private:
    QHostAddress myAddress;
    qint64 myPort;
    QTcpClientConnection *myClientConnection;
};

#endif // QCLIENT_H
