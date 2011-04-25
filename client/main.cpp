#include <QtCore>
#include "qclient.h"

int main(int argc, char *argv[])
{

    QClient a(argc, argv);
    a.connectToServer(QHostAddress("192.168.0.2"), 58988);
    return a.exec();
}
