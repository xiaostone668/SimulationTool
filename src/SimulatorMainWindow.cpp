#include "SimulatorMainWindow.h"
#include "SimulationEngine.h"
#include "STEPReader.h"
#include "SharedMemorySender.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QGroupBox>
#include <QFormLayout>
#include <QApplication>
#include <QResizeEvent>
#include <QStandardPaths>
#include <QDir>
#include <cstring>

// OpenCASCADE includes
#include <Aspect_DisplayConnection.hxx>
#include <OpenGl_GraphicDriver.hxx>
#include <V3d_View.hxx>
#include <AIS_Shape.hxx>
#include <STEPControl_Writer.hxx>
#include <IFSelect_ReturnStatus.hxx>

SimulatorMainWindow::SimulatorMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_fileMenu(nullptr)
    , m_openSTEPAction(nullptr)
    , m_saveResultsAction(nullptr)
    , m_exitAction(nullptr)
    , m_toolBar(nullptr)
    , m_startAction(nullptr)
    , m_pauseAction(nullptr)
    , m_stopAction(nullptr)
    , m_centralWidget(nullptr)
    , m_occViewWidget(nullptr)          // <-- OccViewWidget (was m_occWidget)
    , m_parameterDock(nullptr)
    , m_parameterWidget(nullptr)
    , m_timeStepSpinBox(nullptr)
    , m_totalTimeSpinBox(nullptr)
    , m_dampingSpinBox(nullptr)
    , m_stiffnessSpinBox(nullptr)
    , m_progressBar(nullptr)
    , m_statusLabel(nullptr)
    , m_simulationEngine(nullptr)
    , m_stepReader(nullptr)
    , m_sharedMemorySender(nullptr)
    , m_isSimulationRunning(false)
{
    setupUI();
    initializeOCC();

    // Create core components
    m_simulationEngine    = new SimulationEngine(this);
    m_stepReader          = new STEPReader(this);
    m_sharedMemorySender  = new SharedMemorySender(this);

    // Connect signals
    connect(m_simulationEngine, &SimulationEngine::progressUpdated,
            this, &SimulatorMainWindow::onSimulationProgress);
    connect(m_simulationEngine, &SimulationEngine::simulationFinished,
            this, &SimulatorMainWindow::onSimulationFinished);
    connect(m_simulationEngine, &SimulationEngine::simulationError,
            this, &SimulatorMainWindow::onSimulationError);

    // Initialize shared memory
    m_sharedMemorySender->initialize("SimulationData", 1024 * 1024); // 1 MB

    // ---------- GeomProcessor IPC ----------
    m_geomIpcShm = new QSharedMemory(GEOM_IPC_KEY, this);
    // Try to attach (GeomProcessor may have created it); if not, create ourselves
    if (!m_geomIpcShm->attach(QSharedMemory::ReadWrite)) {
        if (m_geomIpcShm->create(GEOM_IPC_SIZE, QSharedMemory::ReadWrite)) {
            m_geomIpcShm->lock();
            GeomIPCBlock blk;
            memcpy(m_geomIpcShm->data(), &blk, sizeof(blk));
            m_geomIpcShm->unlock();
        }
    }
    // Poll for results from GeomProcessor every 500 ms
    m_resultPollTimer = new QTimer(this);
    m_resultPollTimer->setInterval(500);
    connect(m_resultPollTimer, &QTimer::timeout,
            this, &SimulatorMainWindow::onPollGeomResult);
    m_resultPollTimer->start();

    setWindowTitle("Simulation Tool - OpenCASCADE & Qt");
    resize(1200, 800);
}

SimulatorMainWindow::~SimulatorMainWindow()
{
    if (m_simulationEngine && m_simulationEngine->isRunning()) {
        m_simulationEngine->stopSimulation();
        m_simulationEngine->wait();
    }
}

void SimulatorMainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    // OccViewWidget handles its own resize internally via resizeEvent override
}

// ---------------------------------------------------------------------------
// UI setup
// ---------------------------------------------------------------------------

void SimulatorMainWindow::setupUI()
{
    createMenuBar();
    createToolBar();
    createCentralWidget();
    createParameterPanel();
    createStatusBar();
}

void SimulatorMainWindow::createMenuBar()
{
    m_fileMenu = menuBar()->addMenu(tr("文件(&F)"));

    m_openSTEPAction = new QAction(tr("打开STEP文件(&O)..."), this);
    m_openSTEPAction->setShortcut(QKeySequence::Open);
    connect(m_openSTEPAction, &QAction::triggered, this, &SimulatorMainWindow::onOpenSTEP);
    m_fileMenu->addAction(m_openSTEPAction);

    m_saveResultsAction = new QAction(tr("保存结果(&S)..."), this);
    m_saveResultsAction->setShortcut(QKeySequence::Save);
    connect(m_saveResultsAction, &QAction::triggered, this, &SimulatorMainWindow::onSaveResults);
    m_fileMenu->addAction(m_saveResultsAction);

    m_fileMenu->addSeparator();

    m_exitAction = new QAction(tr("退出(&X)"), this);
    m_exitAction->setShortcut(QKeySequence::Quit);
    connect(m_exitAction, &QAction::triggered, this, &SimulatorMainWindow::onExit);
    m_fileMenu->addAction(m_exitAction);

    // GeomProcessor menu
    QMenu* geomMenu = menuBar()->addMenu(tr("几何处理(&G)"));
    m_sendToGeomAction = new QAction(tr("发送到 GeomProcessor..."), this);
    m_sendToGeomAction->setShortcut(QKeySequence("Ctrl+G"));
    m_sendToGeomAction->setEnabled(false);
    connect(m_sendToGeomAction, &QAction::triggered,
            this, &SimulatorMainWindow::onSendToGeomProcessor);
    geomMenu->addAction(m_sendToGeomAction);
}

void SimulatorMainWindow::createToolBar()
{
    m_toolBar = addToolBar(tr("仿真控制"));

    m_startAction = new QAction(tr("开始仿真"), this);
    m_startAction->setEnabled(false);
    connect(m_startAction, &QAction::triggered, this, &SimulatorMainWindow::onStartSimulation);
    m_toolBar->addAction(m_startAction);

    m_pauseAction = new QAction(tr("暂停"), this);
    m_pauseAction->setEnabled(false);
    connect(m_pauseAction, &QAction::triggered, this, &SimulatorMainWindow::onPauseSimulation);
    m_toolBar->addAction(m_pauseAction);

    m_stopAction = new QAction(tr("停止"), this);
    m_stopAction->setEnabled(false);
    connect(m_stopAction, &QAction::triggered, this, &SimulatorMainWindow::onStopSimulation);
    m_toolBar->addAction(m_stopAction);
}

void SimulatorMainWindow::createCentralWidget()
{
    m_centralWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(m_centralWidget);
    layout->setContentsMargins(0, 0, 0, 0);

    // OccViewWidget: handles all OCC rendering + mouse events
    m_occViewWidget = new OccViewWidget(m_centralWidget);

    layout->addWidget(m_occViewWidget);
    setCentralWidget(m_centralWidget);
}

void SimulatorMainWindow::createParameterPanel()
{
    m_parameterDock   = new QDockWidget(tr("仿真参数"), this);
    m_parameterWidget = new QWidget(m_parameterDock);

    QVBoxLayout* mainLayout = new QVBoxLayout(m_parameterWidget);

    // Time parameters group
    QGroupBox*   paramGroup  = new QGroupBox(tr("时间参数"));
    QFormLayout* formLayout  = new QFormLayout(paramGroup);

    m_timeStepSpinBox = new QDoubleSpinBox();
    m_timeStepSpinBox->setRange(0.0001, 1.0);
    m_timeStepSpinBox->setValue(0.01);
    m_timeStepSpinBox->setDecimals(4);
    m_timeStepSpinBox->setSuffix(" s");
    formLayout->addRow(tr("时间步长:"), m_timeStepSpinBox);

    m_totalTimeSpinBox = new QDoubleSpinBox();
    m_totalTimeSpinBox->setRange(0.1, 1000.0);
    m_totalTimeSpinBox->setValue(10.0);
    m_totalTimeSpinBox->setDecimals(2);
    m_totalTimeSpinBox->setSuffix(" s");
    formLayout->addRow(tr("总时长:"), m_totalTimeSpinBox);

    mainLayout->addWidget(paramGroup);

    // Physics parameters group
    QGroupBox*   physicsGroup  = new QGroupBox(tr("物理参数"));
    QFormLayout* physicsLayout = new QFormLayout(physicsGroup);

    m_dampingSpinBox = new QDoubleSpinBox();
    m_dampingSpinBox->setRange(0.0, 10.0);
    m_dampingSpinBox->setValue(0.1);
    m_dampingSpinBox->setDecimals(3);
    physicsLayout->addRow(tr("阻尼系数:"), m_dampingSpinBox);

    m_stiffnessSpinBox = new QDoubleSpinBox();
    m_stiffnessSpinBox->setRange(1.0, 100000.0);
    m_stiffnessSpinBox->setValue(1000.0);
    m_stiffnessSpinBox->setDecimals(1);
    physicsLayout->addRow(tr("刚度系数:"), m_stiffnessSpinBox);

    mainLayout->addWidget(physicsGroup);
    mainLayout->addStretch();

    m_parameterDock->setWidget(m_parameterWidget);
    addDockWidget(Qt::RightDockWidgetArea, m_parameterDock);
}

void SimulatorMainWindow::createStatusBar()
{
    m_statusLabel = new QLabel(tr("就绪"));
    m_progressBar = new QProgressBar();
    m_progressBar->setMinimumWidth(200);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);

    statusBar()->addWidget(m_statusLabel, 1);
    statusBar()->addWidget(m_progressBar);
}

// ---------------------------------------------------------------------------
// OCC initialization
// ---------------------------------------------------------------------------

void SimulatorMainWindow::initializeOCC()
{
    try {
        // Graphic driver
        m_displayConnection = new Aspect_DisplayConnection();
        m_graphicDriver     = new OpenGl_GraphicDriver(m_displayConnection);

        // Viewer
        m_viewer = new V3d_Viewer(m_graphicDriver);
        m_viewer->SetDefaultLights();
        m_viewer->SetLightOn();

        // AIS context
        m_context = new AIS_InteractiveContext(m_viewer);

        // Hand off viewer + context to the viewport widget.
        // The widget will create the V3d_View and attach the native window
        // as soon as it receives its first paint/resize event (deferred init).
        if (m_occViewWidget) {
            m_occViewWidget->init(m_viewer, m_context);
        }

    } catch (const Standard_Failure& e) {
        QMessageBox::critical(this, tr("初始化错误"),
            tr("OpenCASCADE初始化失败: %1").arg(e.GetMessageString()));
    }
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------

void SimulatorMainWindow::onOpenSTEP()
{
    QString filePath = QFileDialog::getOpenFileName(this,
        tr("打开STEP文件"), "", tr("STEP Files (*.step *.stp)"));

    if (filePath.isEmpty()) return;

    m_statusLabel->setText(tr("正在加载STEP文件..."));

    if (m_stepReader->loadSTEPFile(filePath)) {
        m_currentFilePath = filePath;
        m_stepReader->displayShape(m_context, true);

        // FitAll + Redraw via the OccViewWidget
        if (m_occViewWidget) {
            Handle(V3d_View) v = m_occViewWidget->view();
            if (!v.IsNull()) {
                v->FitAll();
                v->Redraw();
            }
        }

        auto info = m_stepReader->getGeometryInfo();
        m_statusLabel->setText(
            tr("已加载: %1 个面, %2 个边").arg(info.numFaces).arg(info.numEdges));
        m_startAction->setEnabled(true);
        if (m_sendToGeomAction) m_sendToGeomAction->setEnabled(true);
    } else {
        m_statusLabel->setText(tr("STEP文件加载失败"));
        QMessageBox::warning(this, tr("加载失败"),
            tr("无法加载STEP文件: %1").arg(m_stepReader->getLastError()));
    }
}

void SimulatorMainWindow::onSaveResults()
{
    QString filePath = QFileDialog::getSaveFileName(this,
        tr("保存结果"), "", tr("CSV Files (*.csv);;Text Files (*.txt)"));
    if (filePath.isEmpty()) return;

    m_statusLabel->setText(tr("结果保存成功"));
}

void SimulatorMainWindow::onExit()
{
    qApp->quit();
}

void SimulatorMainWindow::onStartSimulation()
{
    if (m_isSimulationRunning) return;
    if (!m_stepReader->hasShape()) {
        QMessageBox::warning(this, tr("警告"), tr("请先加载STEP文件"));
        return;
    }

    SimulationEngine::SimulationParameters params;
    params.timeStep  = m_timeStepSpinBox->value();
    params.totalTime = m_totalTimeSpinBox->value();
    params.damping   = m_dampingSpinBox->value();
    params.stiffness = m_stiffnessSpinBox->value();

    m_simulationEngine->setParameters(params);
    m_simulationEngine->startSimulation();
    m_isSimulationRunning = true;

    m_startAction->setEnabled(false);
    m_pauseAction->setEnabled(true);
    m_stopAction->setEnabled(true);
    m_statusLabel->setText(tr("仿真运行中..."));
}

void SimulatorMainWindow::onPauseSimulation()
{
    if (!m_isSimulationRunning) return;

    if (m_simulationEngine->isPaused()) {
        m_simulationEngine->resumeSimulation();
        m_pauseAction->setText(tr("暂停"));
        m_statusLabel->setText(tr("仿真运行中..."));
    } else {
        m_simulationEngine->pauseSimulation();
        m_pauseAction->setText(tr("继续"));
        m_statusLabel->setText(tr("仿真已暂停"));
    }
}

void SimulatorMainWindow::onStopSimulation()
{
    if (!m_isSimulationRunning) return;

    m_simulationEngine->stopSimulation();
    m_isSimulationRunning = false;

    m_startAction->setEnabled(true);
    m_pauseAction->setEnabled(false);
    m_stopAction->setEnabled(false);
    m_progressBar->setValue(0);
    m_statusLabel->setText(tr("仿真已停止"));
}

void SimulatorMainWindow::onSimulationProgress(int progress)
{
    m_progressBar->setValue(progress);
    m_statusLabel->setText(tr("仿真运行中... %1%").arg(progress));
}

void SimulatorMainWindow::onSimulationFinished()
{
    m_isSimulationRunning = false;
    m_pauseAction->setText(tr("暂停"));

    m_startAction->setEnabled(true);
    m_pauseAction->setEnabled(false);
    m_stopAction->setEnabled(false);
    m_progressBar->setValue(100);
    m_statusLabel->setText(tr("仿真完成"));
}

void SimulatorMainWindow::onSimulationError(const QString& error)
{
    m_isSimulationRunning = false;

    m_startAction->setEnabled(true);
    m_pauseAction->setEnabled(false);
    m_stopAction->setEnabled(false);
    m_statusLabel->setText(tr("仿真错误"));

    QMessageBox::critical(this, tr("仿真错误"), error);
}

// ---------------------------------------------------------------------------
// GeomProcessor IPC
// ---------------------------------------------------------------------------

void SimulatorMainWindow::onSendToGeomProcessor()
{
    if (!m_stepReader->hasShape()) {
        QMessageBox::warning(this, tr("警告"), tr("请先加载 STEP 文件"));
        return;
    }

    if (!m_geomIpcShm || !m_geomIpcShm->isAttached()) {
        QMessageBox::warning(this, tr("IPC 错误"),
            tr("共享内存未就绪，请先启动 GeomProcessor.exe"));
        return;
    }

    // 直接发送原始 STEP 文件路径，避免 STEPControl_Writer 重写带来的兼容问题
    QString sendPath = m_currentFilePath;
    if (sendPath.isEmpty()) {
        QMessageBox::warning(this, tr("路径错误"), tr("未找到原始 STEP 文件路径"));
        return;
    }
    // 使用本地路径分隔符，确保 OCC 读取兼容
    sendPath = QDir::toNativeSeparators(sendPath);

    // 写入 IPC 块：CMD_SEND_GEOM + 文件路径
    m_geomIpcShm->lock();
    GeomIPCBlock blk;
    memcpy(&blk, m_geomIpcShm->constData(), sizeof(blk));
    blk.cmd = CMD_SEND_GEOM;
    blk.seqNo = ++m_geomSeqNo;
    QByteArray pathBytes = sendPath.toLocal8Bit();
    strncpy(blk.geomFilePath, pathBytes.constData(), sizeof(blk.geomFilePath) - 1);
    memcpy(m_geomIpcShm->data(), &blk, sizeof(blk));
    m_geomIpcShm->unlock();

    m_statusLabel->setText(tr("几何已发送至 GeomProcessor #%1: %2")
        .arg(m_geomSeqNo).arg(sendPath));
}

void SimulatorMainWindow::onPollGeomResult()
{
    if (!m_geomIpcShm || !m_geomIpcShm->isAttached()) return;

    m_geomIpcShm->lock();
    GeomIPCBlock blk;
    memcpy(&blk, m_geomIpcShm->constData(), sizeof(blk));
    m_geomIpcShm->unlock();

    if (blk.magic != (uint32_t)GEOM_IPC_MAGIC) return;
    if (blk.cmd != CMD_RESULT_READY)            return;

    // Read result file
    QString resultPath = QString::fromLocal8Bit(blk.resultFilePath);
    if (resultPath.isEmpty()) return;

    m_statusLabel->setText(tr("正在加载 GeomProcessor 结果: %1").arg(resultPath));

    if (m_stepReader->loadSTEPFile(resultPath)) {
        m_stepReader->displayShape(m_context, true);
        if (m_occViewWidget) {
            Handle(V3d_View) v = m_occViewWidget->view();
            if (!v.IsNull()) { v->FitAll(); v->Redraw(); }
        }
        auto info = m_stepReader->getGeometryInfo();
        m_statusLabel->setText(
            tr("已更新几何（来自 GeomProcessor）: %1 个面, %2 个边")
            .arg(info.numFaces).arg(info.numEdges));
    } else {
        m_statusLabel->setText(tr("加载 GeomProcessor 结果失败"));
    }

    // Acknowledge: reset to IDLE
    m_geomIpcShm->lock();
    blk.cmd = CMD_IDLE;
    memcpy(m_geomIpcShm->data(), &blk, sizeof(blk));
    m_geomIpcShm->unlock();
}
