#ifndef WINDOWUTILS_H
#define WINDOWUTILS_H

#include <QWidget>
#include <QScreen>

/**
 * @brief Utility class for common window operations
 * 
 * Provides cross-platform window management functionality like 
 * window positioning, sizing, and other display-related operations.
 */
class WindowUtils
{
public:
    /**
     * @brief Centers a window on its current screen
     * @param window The window to be centered
     */
    static void centerWindowOnScreen(QWidget *window);
};

#endif // WINDOWUTILS_H 