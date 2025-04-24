#include "appintegration.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QFileInfo>
#include <QStandardPaths>
#include <QProcess>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QTextStream>

namespace {
    /**
     * @brief Updates a field in desktop file if the value is different
     * @param desktopFilePath Path to the desktop file
     * @param fieldName Name of the field to update
     * @param fieldValue New value for the field
     */
    void updateDesktopField(const QString& desktopFilePath, 
                           const QString& fieldName, 
                           const QString& fieldValue)
    {
#ifdef Q_OS_LINUX
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
#else
        // Non-Linux platforms - do nothing
        Q_UNUSED(desktopFilePath);
        Q_UNUSED(fieldName);
        Q_UNUSED(fieldValue);
#endif
    }

    /**
     * @brief Refreshes system caches for desktop files and icons
     */
    void refreshSystemCache()
    {
#ifdef Q_OS_LINUX
        // Update desktop database
        QProcess::execute("update-desktop-database", QStringList() << QDir::homePath() + "/.local/share/applications");
        
        // Ensure icon directories exist for cache update
        QDir().mkpath(QDir::homePath() + "/.local/share/icons/hicolor");
        
        // Refresh icon cache - try different commands based on available tools
        QProcess::execute("gtk-update-icon-cache", QStringList() << "-f" << "-t" << QDir::homePath() + "/.local/share/icons");
        QProcess::execute("xdg-icon-resource", QStringList() << "forceupdate");
        
        // Notify desktop environment of changes - works for many desktop environments
        QProcess::execute("dbus-send", QStringList() 
            << "--session" 
            << "--dest=org.freedesktop.DBus" 
            << "--type=method_call"
            << "/org/freedesktop/DBus" 
            << "org.freedesktop.DBus.ReloadConfig");
#endif
    }
}

void AppIntegration::loadApplicationIcon(QApplication& app)
{
    // Try to load icon from resource
    QIcon icon(":/app-icon-png");
    if (!icon.isNull()) {
        app.setWindowIcon(icon);
        qDebug() << "Icon loaded from resource";
    }

#ifdef Q_OS_LINUX
    // Setup desktop entry for Linux
    setupDesktopEntry();
#endif
}

#ifdef Q_OS_LINUX
void AppIntegration::setupDesktopEntry()
{
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
            
            // Force system to reload desktop files and refresh icon cache
            // Only do this on first run (when desktop file doesn't exist)
            qDebug() << "First run detected - refreshing desktop database and icon cache";
            refreshSystemCache();
        }
    }
    // Whether desktop file is newly created or exists already, check and update Exec field if necessary
    updateDesktopField(userDesktopFilePath, "Exec", appImagePath);
}
#endif 