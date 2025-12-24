#include <QApplication>

#include "DemoWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    DemoWindow w;
    w.show();

    return app.exec();
}
