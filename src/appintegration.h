#ifndef APPINTEGRATION_H
#define APPINTEGRATION_H

#include <QString>
#include <QIcon>
#include <QApplication>

/**
 * @brief Application desktop integration manager
 * 
 * Handles platform-specific desktop integration features like:
 * - Application icon management
 * - Desktop file creation (Linux)
 * - Windows/macOS specific icon handling
 * 
 * Uses conditional compilation to include only platform-relevant code.
 */
class AppIntegration
{
public:
    /**
     * @brief Loads application icon from resources or file system
     */
    static void loadApplicationIcon(QApplication& app);

#ifdef Q_OS_LINUX
    /**
     * @brief Sets up desktop entry file for Linux desktop environment
     */
    static void setupDesktopEntry();
#endif
};

#endif // APPINTEGRATION_H 