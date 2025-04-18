#include <QApplication>
#include <QMainWindow>
#include <QScreen>
#include <QIcon>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QRegularExpression>

// Application settings
const QString APP_TITLE = "Long View";
const QString ORGANIZATION_NAME = "basgeekball";
const QString ORGANIZATION_DOMAIN = "com.basgeekball";
const QString APP_DESKTOP_FILE = "longview.desktop";
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

// Function prototypes
void centerWindowOnScreen(QWidget *window);
void updateDesktopField(const QString& desktopFilePath, const QString& fieldName, const QString& fieldValue);
void setupAppDesktopEntry();

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setOrganizationName(ORGANIZATION_NAME);
    app.setOrganizationDomain(ORGANIZATION_DOMAIN);
    app.setApplicationName(APP_TITLE);
    
    app.setDesktopFileName(APP_DESKTOP_FILE);
    
    // Make sure .desktop file exists in user's home directory with correct path
    setupAppDesktopEntry();
    
    // Try to load icon from resource
    QIcon appIcon(":/app-icon-png");
    if (!appIcon.isNull()) {
        app.setWindowIcon(appIcon);
    }
    
    // Create main window
    QMainWindow mainWindow;
    mainWindow.setWindowTitle(APP_TITLE);
    mainWindow.resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    
    // Center and show window
    centerWindowOnScreen(&mainWindow);
    
    return app.exec();
}

/**
 * Centers a window on its current screen and shows it
 * @param window The window to be centered
 */
void centerWindowOnScreen(QWidget *window) {
    // Check if window is visible, show it if not
    bool isHidden = !window->isVisible();
    if (isHidden) {
        window->show();
    }
    
    // Get screen geometry and center the window
    QScreen *screen = window->screen();
    QRect screenGeometry = screen->geometry();
    window->setGeometry(
        screenGeometry.x() + (screenGeometry.width() - window->width()) / 2,
        screenGeometry.y() + (screenGeometry.height() - window->height()) / 2,
        window->width(),
        window->height()
    );
}

/**
 * Updates a field in desktop file if the value is different
 * @param desktopFilePath Path to the desktop file
 * @param fieldName Name of the field to update
 * @param fieldValue New value for the field
 */
void updateDesktopField(const QString& desktopFilePath, const QString& fieldName, const QString& fieldValue) {
    QFile file(desktopFilePath);
    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
        QString content = file.readAll();
        file.close();
        
        // Match field and replace with new value if different
        QRegularExpression regex(fieldName + "=(.*)");
        QString replacement = fieldName + "=" + fieldValue;
        QRegularExpressionMatch match = regex.match(content);
        QString oldValue = match.hasMatch() ? match.captured(1) : "";
        if (oldValue != fieldValue) {
            content.replace(regex, replacement);
        } else {
            qDebug() << "Field value is the same, skipping update for" << fieldName;
            return;
        }
        
        // Write back to file
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            file.write(content.toUtf8());
            file.close();
            qDebug() << "Updated desktop file:" << desktopFilePath;
            qDebug() << "Field name:" << fieldName;
            qDebug() << "Old value:" << oldValue;
            qDebug() << "New value:" << fieldValue;
        } else {
            qDebug() << "Failed to update desktop file:" << desktopFilePath;
        }
    } else {
        qDebug() << "Failed to open desktop file for reading:" << desktopFilePath;
    }
}

/**
 * Sets up desktop entry file for the application integration with desktop environment
 */
void setupAppDesktopEntry() {
    // Check if running from AppImage
    const char* appImage = getenv("APPIMAGE");
    if (!appImage) {
        // Not running from AppImage, nothing to do
        return;
    }
    
    // Get paths
    QString appImagePath(appImage);
    QString appDir = QCoreApplication::applicationDirPath();
    QString appIconPath = appDir + "/../icons/longview.png";
    QString appDesktopPath = appDir + "/../applications/longview.desktop";
    const QString USER_LOCAL_DATA_DIR = QDir::homePath() + "/.local/share";
    QString userDesktopFilePath = USER_LOCAL_DATA_DIR + "/applications/longview.desktop";
    QString userIconDir = USER_LOCAL_DATA_DIR + "/icons";
    QString userIconPath = userIconDir + "/longview.png";
    
    // Ensure icon directory exists
    QDir().mkpath(userIconDir);

    // Try to copy icon from AppImage to user directory
    if (QFile::copy(appIconPath, userIconPath)) {
        qDebug() << "Icon copied from:" << appIconPath << "to:" << userIconPath;
    }
    
    // Check if desktop file exists
    QFile desktopFile(userDesktopFilePath);
    if (!desktopFile.exists()) {
        // Desktop file doesn't exist, copy from template and update
        QDir().mkpath(QFileInfo(userDesktopFilePath).path());
        if (QFile::copy(appDesktopPath, userDesktopFilePath)) {
            qDebug() << "Desktop file copied from:" << appDesktopPath << "to:" << userDesktopFilePath;
            // Update Icon fields in the new desktop file
            updateDesktopField(userDesktopFilePath, "Icon", userIconPath);
        }
    }
    // Whether desktop file is newly created or exists already, check and update Exec field if necessary
    updateDesktopField(userDesktopFilePath, "Exec", appImagePath);
}