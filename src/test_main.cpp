#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QStatusBar>
#include <QLabel>
#include <QDebug>

class SimpleMainWindow : public QMainWindow
{
    Q_OBJECT
public:
    SimpleMainWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
    {
        setWindowTitle("Simulation Tool - Simple Test");
        resize(800, 600);
        
        // Create menu bar
        QMenu *fileMenu = menuBar()->addMenu("文件(&F)");
        QAction *openAction = fileMenu->addAction("打开(&O)");
        QAction *exitAction = fileMenu->addAction("退出(&X)");
        
        connect(openAction, &QAction::triggered, this, &SimpleMainWindow::onOpen);
        connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
        
        // Create status bar
        QLabel *statusLabel = new QLabel("就绪");
        statusBar()->addWidget(statusLabel);
        
        qDebug() << "SimpleMainWindow created successfully";
    }
    
private slots:
    void onOpen()
    {
        QMessageBox::information(this, "打开", "打开功能测试成功！");
        qDebug() << "Open action triggered";
    }
};

int main(int argc, char *argv[])
{
    qDebug() << "Starting simple test application...";
    
    QApplication app(argc, argv);
    app.setApplicationName("Simulation Tool Test");
    app.setApplicationVersion("1.0.0");
    
    SimpleMainWindow window;
    window.show();
    
    qDebug() << "Application running...";
    
    return app.exec();
}

#include "test_main.moc"