#include "windowutils.h"
#include <QDebug>

void WindowUtils::centerWindowOnScreen(QWidget *window)
{
    if (!window) {
        qWarning() << "Cannot center null window";
        return;
    }
    
    // Check if window is visible, show it if not
    if (!window->isVisible()) {
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