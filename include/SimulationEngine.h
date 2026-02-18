#ifndef SIMULATIONENGINE_H
#define SIMULATIONENGINE_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <atomic>
#include <vector>

/**
 * @brief Simulation computation engine running in a separate thread
 * 
 * This class handles the core simulation calculations independently
 * from the UI thread. It supports:
 * - Start/Pause/Stop control
 * - Progress reporting
 * - Time-stepping simulation
 * - Thread-safe parameter updates
 */
class SimulationEngine : public QThread
{
    Q_OBJECT

public:
    /**
     * @brief Simulation parameters structure
     */
    struct SimulationParameters
    {
        double timeStep;        // Time step size (seconds)
        double totalTime;       // Total simulation time (seconds)
        double damping;         // Damping coefficient
        double stiffness;       // Stiffness coefficient
        int maxIterations;      // Maximum iterations per step
        double tolerance;       // Convergence tolerance

        SimulationParameters()
            : timeStep(0.01)
            , totalTime(10.0)
            , damping(0.1)
            , stiffness(1000.0)
            , maxIterations(100)
            , tolerance(1e-6)
        {}
    };

    /**
     * @brief Simulation state structure
     */
    struct SimulationState
    {
        double currentTime;
        int currentStep;
        int totalSteps;
        std::vector<double> positions;
        std::vector<double> velocities;
        std::vector<double> accelerations;

        SimulationState()
            : currentTime(0.0)
            , currentStep(0)
            , totalSteps(0)
        {}
    };

    explicit SimulationEngine(QObject *parent = nullptr);
    ~SimulationEngine();

    // Control methods
    void startSimulation();
    void pauseSimulation();
    void stopSimulation();
    void resumeSimulation();

    // Parameter management
    void setParameters(const SimulationParameters& params);
    SimulationParameters getParameters() const;

    // State queries
    bool isRunning() const { return m_isRunning; }
    bool isPaused() const { return m_isPaused; }
    SimulationState getCurrentState() const;

signals:
    void progressUpdated(int progress);
    void simulationFinished();
    void simulationError(const QString& error);
    void stateUpdated(const SimulationState& state);

protected:
    void run() override;

private:
    // Simulation computation methods
    void initializeSimulation();
    void performTimeStep();
    void computeForces(std::vector<double>& forces);
    void integrateMotion(const std::vector<double>& forces);
    void checkConvergence();
    void finalizeSimulation();

    // Thread synchronization
    mutable QMutex m_mutex;
    QWaitCondition m_pauseCondition;

    // Simulation parameters and state
    SimulationParameters m_parameters;
    SimulationState m_state;

    // Control flags
    std::atomic<bool> m_isRunning;
    std::atomic<bool> m_isPaused;
    std::atomic<bool> m_shouldStop;

    // Internal state
    int m_progressPercent;
};

#endif // SIMULATIONENGINE_H
