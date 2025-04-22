#include <QApplication>
#include <QMainWindow>
#include <QScreen>
#include "windowutils.h"
#include "appintegration.h"

// Application settings
const QString APP_TITLE = "Long View";
const QString ORGANIZATION_NAME = "basgeekball";
const QString ORGANIZATION_DOMAIN = "com.basgeekball";
const QString APP_DESKTOP_FILE = "longview.desktop";
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application information
    app.setOrganizationName(ORGANIZATION_NAME);
    app.setOrganizationDomain(ORGANIZATION_DOMAIN);
    app.setApplicationName(APP_TITLE);
    app.setDesktopFileName(APP_DESKTOP_FILE);
    
    // Load application icon
    AppIntegration::loadApplicationIcon(app);
    
    // Create main window
    QMainWindow mainWindow;
    mainWindow.setWindowTitle(APP_TITLE);
    mainWindow.resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Center and show window
    WindowUtils::centerWindowOnScreen(&mainWindow);
    
    return app.exec();
}