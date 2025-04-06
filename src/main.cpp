#include <QApplication>
#include <QMainWindow>
#include <QScreen>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("LongView");
    mainWindow.resize(1024, 768);
    
    // Center the window on the screen
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    mainWindow.move(
        (screenGeometry.width() - mainWindow.width()) / 2,
        (screenGeometry.height() - mainWindow.height()) / 2
    );
    
    mainWindow.show();
    
    return app.exec();
}