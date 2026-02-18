#include "SimulatorMainWindow.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDateTime>

// Function to log to file
void logToFile(const QString& message)
{
    QFile file("simulation_tool_log.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&file);
        stream << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz") << " " << message << "\n";
        file.close();
    }
}

int main(int argc, char *argv[])
{
    // Redirect qDebug to file
    logToFile("Starting application...");

    // Set OpenGL format for OpenCASCADE
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(format);
    logToFile("OpenGL format set");

    QApplication app(argc, argv);
    logToFile("QApplication created");

    // Set application information
    app.setApplicationName("Simulation Tool");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("MyAICAD");
    logToFile("Application info set");

    try {
        logToFile("Creating main window...");
        // Create and show main window
        SimulatorMainWindow mainWindow;
        logToFile("Main window created");
        mainWindow.show();
        logToFile("Main window shown");
        logToFile("Entering event loop...");
        return app.exec();
    } catch (const std::exception& e) {
        QString errorMsg = QString("Exception caught: %1").arg(e.what());
        logToFile(errorMsg);
        QMessageBox::critical(nullptr, "Error", errorMsg);
        return 1;
    } catch (...) {
        logToFile("Unknown exception caught");
        QMessageBox::critical(nullptr, "Error", "Unknown exception caught");
        return 1;
    }
}
