#include "../common/workingdirectory/WorkingDirectoryManager.h"
#include "../common/workingdirectory/DialogOpenConfig.h"
#include "../common/types/types.h"

#include "MainWindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    qRegisterMetaType<QList<QStringList>>();
    qRegisterMetaType<QSet<QString>>();

    QCoreApplication::setOrganizationName("Icinger Power");
    QCoreApplication::setOrganizationDomain("ecomelitepro.com");
    QCoreApplication::setApplicationName("Amazon Order Maker");
    QApplication a(argc, argv);
    WorkingDirectoryManager::instance()->installDarkOrangePalette();
    DialogOpenConfig dialog;
    dialog.exec();
    if (dialog.wasRejected())
    {
        return 0;
    }
    MainWindow w;
    w.show();
    return a.exec();
}
