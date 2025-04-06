#include <QApplication>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    QMainWindow mainWindow;
    mainWindow.setWindowTitle("LongView");
    mainWindow.resize(1024, 768);
    mainWindow.show();
    
    return app.exec();
} 