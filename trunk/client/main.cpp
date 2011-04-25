#include <QtCore>
#include "qclient.h"

int main(int argc, char *argv[])
{

    QClient a(argc, argv);
    a.connectToServer(QHostAddress("193.33.144.152"), 58988);
    return a.exec();
}
