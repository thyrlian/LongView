#include <QApplication>
#include <QMainWindow>
#include <QScreen>

// Application settings
const QString APP_TITLE = "Long View";
const int WINDOW_WIDTH = 1024;
const int WINDOW_HEIGHT = 768;

// Function prototypes
void centerWindowOnScreen(QWidget *window);

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QMainWindow mainWindow;
    mainWindow.setWindowTitle(APP_TITLE);
    mainWindow.resize(WINDOW_WIDTH, WINDOW_HEIGHT);
    centerWindowOnScreen(&mainWindow);
    
    return app.exec();
}

/**
 * Centers a window on its current screen
 * @param window The window to be centered
 */
void centerWindowOnScreen(QWidget *window) {
    // Check if window is visible, show it if not
    bool wasHidden = !window->isVisible();
    if (wasHidden) {
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