#include "BasicFunction.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QBasicFunction w;
    w.show();

    return a.exec();
}
