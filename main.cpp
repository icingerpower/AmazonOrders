#include "../common/workingdirectory/WorkingDirectoryManager.h"

#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Icinger Power");
    QCoreApplication::setOrganizationDomain("ecomelitepro.com");
    QCoreApplication::setApplicationName("Amazon Order Maker");
    QApplication a(argc, argv);
    WorkingDirectoryManager::instance()->installDarkOrangePalette();
    MainWindow w;
    w.show();
    return a.exec();
}
