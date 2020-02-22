#include <QApplication>
#include <QObject>
#include "nayk/AppCore"
#include "canvas.h"
//==================================================================================================
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    if(!app_core::initializeApplication( "desktopWidget",
                                         "nayk",
                                         "nayk1982.github.io"
                                         )) {
        return 0;
    }

    Canvas canvas;
    canvas.setObjectName("canvasObject");
    QObject::connect(&canvas, &Canvas::quit, &a, &QApplication::quit, Qt::QueuedConnection);
    canvas.show();

    return a.exec();
}
//==================================================================================================
