#ifndef SIMULATORMAINWINDOW_H
#define SIMULATORMAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QDockWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QProgressBar>
#include <QPushButton>
#include <QAction>
#include <QResizeEvent>

// OpenCASCADE includes
#include <AIS_InteractiveContext.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>

#include "OccViewWidget.h"
#include "GeomIPC.h"

#include <QSharedMemory>
#include <QTimer>

// Forward declarations
class SimulationEngine;
class STEPReader;
class SharedMemorySender;

/**
 * @brief Main window class for the simulation tool
 * 
 * This class provides the main user interface including:
 * - Menu bar with File operations
 * - Toolbar with simulation controls
 * - 3D view area using OpenCASCADE
 * - Parameter panel for simulation settings
 * - Status bar for progress display
 */
class SimulatorMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SimulatorMainWindow(QWidget *parent = nullptr);
    ~SimulatorMainWindow();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    // Menu actions
    void onOpenSTEP();
    void onSaveResults();
    void onExit();

    // Toolbar actions
    void onStartSimulation();
    void onPauseSimulation();
    void onStopSimulation();

    // Simulation engine callbacks
    void onSimulationProgress(int progress);
    void onSimulationFinished();
    void onSimulationError(const QString& error);

    // GeomProcessor IPC
    void onSendToGeomProcessor();
    void onPollGeomResult();

private:
    // UI setup methods
    void setupUI();
    void createMenuBar();
    void createToolBar();
    void createCentralWidget();
    void createParameterPanel();
    void createStatusBar();

    // OpenCASCADE initialization
    void initializeOCC();

    // Menu bar
    QMenu* m_fileMenu;
    QAction* m_openSTEPAction;
    QAction* m_saveResultsAction;
    QAction* m_exitAction;

    // Toolbar
    QToolBar* m_toolBar;
    QAction* m_startAction;
    QAction* m_pauseAction;
    QAction* m_stopAction;

    // Central widget - 3D View
    QWidget*       m_centralWidget;
    OccViewWidget* m_occViewWidget;   ///< native OCC viewport (replaces m_occWidget)

    // OpenCASCADE components (viewer + context owned here; view owned by OccViewWidget)
    Handle(Aspect_DisplayConnection) m_displayConnection;
    Handle(OpenGl_GraphicDriver)     m_graphicDriver;
    Handle(V3d_Viewer)               m_viewer;
    Handle(AIS_InteractiveContext)   m_context;

    // Parameter panel
    QDockWidget* m_parameterDock;
    QWidget* m_parameterWidget;
    QDoubleSpinBox* m_timeStepSpinBox;
    QDoubleSpinBox* m_totalTimeSpinBox;
    QDoubleSpinBox* m_dampingSpinBox;
    QDoubleSpinBox* m_stiffnessSpinBox;

    // Status bar
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;

    // 3D 视图左下角几何信息悬浮标签
    QLabel* m_geomInfoLabel = nullptr;
    void    updateGeomInfoLabel(const QString& text);

    // Core components
    SimulationEngine* m_simulationEngine;
    STEPReader* m_stepReader;
    SharedMemorySender* m_sharedMemorySender;

    // State
    QString m_currentFilePath;
    bool m_isSimulationRunning;

    // GeomProcessor IPC
    QSharedMemory* m_geomIpcShm       = nullptr;
    QTimer*        m_resultPollTimer  = nullptr;
    uint32_t       m_geomSeqNo        = 0;
    QAction*       m_sendToGeomAction = nullptr;
};

#endif // SIMULATORMAINWINDOW_H
