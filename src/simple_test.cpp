#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QStatusBar>
#include <QLabel>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

void logToFile(const QString& message)
{
    QFile file("simple_test_log.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&file);
        stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << " " << message << "\n";
        file.close();
    }
}

class SimpleMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    SimpleMainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        logToFile("SimpleMainWindow constructor start");
        
        setWindowTitle("Simulation Tool - Simple Test");
        resize(800, 600);
        
        logToFile("Window title and size set");
        
        // Create menu bar
        QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
        logToFile("Menu bar created");
        
        QAction *openAction = fileMenu->addAction("打开(&O)");
        QAction *exitAction = fileMenu->addAction("退出(&X)");
        logToFile("Actions created");
        
        connect(openAction, &QAction::triggered, this, &SimpleMainWindow::onOpen);
        connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
        logToFile("Connections made");
        
        // Create status bar
        QLabel *statusLabel = new QLabel("就绪");
        statusBar()->addWidget(statusLabel);
        logToFile("Status bar created");
        
        logToFile("SimpleMainWindow constructor end");
    }
    
private slots:
    void onOpen()
    {
        logToFile("Open action triggered");
        QMessageBox::information(this, "打开", "打开功能测试成功！");
        logToFile("Message box shown");
    }
};

int main(int argc, char *argv[])
{
    logToFile("Starting simple test application...");
    
    QApplication app(argc, argv);
    logToFile("QApplication created");
    
    app.setApplicationName("Simulation Tool Test");
    app.setApplicationVersion("1.0.0");
    logToFile("Application info set");
    
    SimpleMainWindow window;
    logToFile("Main window created");
    
    window.show();
    logToFile("Main window shown");
    
    logToFile("Application running...");
    
    return app.exec();
}