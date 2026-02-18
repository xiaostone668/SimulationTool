#include "SimulationEngine.h"
#include <QThread>
#include <QMutexLocker>
#include <cmath>

SimulationEngine::SimulationEngine(QObject *parent)
    : QThread(parent)
    , m_isRunning(false)
    , m_isPaused(false)
    , m_shouldStop(false)
    , m_progressPercent(0)
{
}

SimulationEngine::~SimulationEngine()
{
    if (isRunning()) {
        stopSimulation();
        wait();
    }
}

void SimulationEngine::startSimulation()
{
    if (m_isRunning) {
        return;
    }

    m_shouldStop = false;
    m_isPaused = false;
    start();
}

void SimulationEngine::pauseSimulation()
{
    QMutexLocker locker(&m_mutex);
    m_isPaused = true;
}

void SimulationEngine::stopSimulation()
{
    m_shouldStop = true;
    m_isPaused = false;
    m_pauseCondition.wakeAll();
}

void SimulationEngine::resumeSimulation()
{
    QMutexLocker locker(&m_mutex);
    m_isPaused = false;
    m_pauseCondition.wakeAll();
}

void SimulationEngine::setParameters(const SimulationParameters& params)
{
    QMutexLocker locker(&m_mutex);
    m_parameters = params;
}

SimulationEngine::SimulationParameters SimulationEngine::getParameters() const
{
    QMutexLocker locker(&m_mutex);
    return m_parameters;
}

SimulationEngine::SimulationState SimulationEngine::getCurrentState() const
{
    QMutexLocker locker(&m_mutex);
    return m_state;
}

void SimulationEngine::run()
{
    m_isRunning = true;

    try {
        initializeSimulation();

        while (m_state.currentStep < m_state.totalSteps && !m_shouldStop) {
            // Check for pause
            {
                QMutexLocker locker(&m_mutex);
                while (m_isPaused && !m_shouldStop) {
                    m_pauseCondition.wait(&m_mutex);
                }
            }

            if (m_shouldStop) {
                break;
            }

            // Perform simulation step
            performTimeStep();

            // Update progress
            int progress = static_cast<int>(
                (static_cast<double>(m_state.currentStep) / m_state.totalSteps) * 100.0
            );

            if (progress != m_progressPercent) {
                m_progressPercent = progress;
                emit progressUpdated(progress);
            }

            // Emit state update
            emit stateUpdated(m_state);

            // Small delay to prevent CPU overload
            msleep(1);
        }

        finalizeSimulation();

        if (!m_shouldStop) {
            emit simulationFinished();
        }
    }
    catch (const std::exception& e) {
        emit simulationError(QString("Simulation error: %1").arg(e.what()));
    }
    catch (...) {
        emit simulationError("Unknown simulation error occurred");
    }

    m_isRunning = false;
}

void SimulationEngine::initializeSimulation()
{
    QMutexLocker locker(&m_mutex);

    // Calculate total steps
    m_state.totalSteps = static_cast<int>(m_parameters.totalTime / m_parameters.timeStep);
    m_state.currentStep = 0;
    m_state.currentTime = 0.0;

    // Initialize state vectors (example: 10 degrees of freedom)
    const int numDOF = 10;
    m_state.positions.resize(numDOF, 0.0);
    m_state.velocities.resize(numDOF, 0.0);
    m_state.accelerations.resize(numDOF, 0.0);

    // Set initial conditions (example: small perturbation)
    for (int i = 0; i < numDOF; ++i) {
        m_state.positions[i] = 0.01 * std::sin(i * 0.5);
    }

    m_progressPercent = 0;
}

void SimulationEngine::performTimeStep()
{
    QMutexLocker locker(&m_mutex);

    // Compute forces
    std::vector<double> forces(m_state.positions.size());
    computeForces(forces);

    // Integrate motion
    integrateMotion(forces);

    // Update time and step
    m_state.currentTime += m_parameters.timeStep;
    m_state.currentStep++;
}

void SimulationEngine::computeForces(std::vector<double>& forces)
{
    // Simple spring-damper system
    const size_t n = m_state.positions.size();

    for (size_t i = 0; i < n; ++i) {
        // Spring force: F = -k * x
        double springForce = -m_parameters.stiffness * m_state.positions[i];

        // Damping force: F = -c * v
        double dampingForce = -m_parameters.damping * m_state.velocities[i];

        // Total force
        forces[i] = springForce + dampingForce;

        // Add coupling with neighbors (if applicable)
        if (i > 0) {
            forces[i] += 0.1 * m_parameters.stiffness * (m_state.positions[i-1] - m_state.positions[i]);
        }
        if (i < n - 1) {
            forces[i] += 0.1 * m_parameters.stiffness * (m_state.positions[i+1] - m_state.positions[i]);
        }
    }
}

void SimulationEngine::integrateMotion(const std::vector<double>& forces)
{
    // Simple Euler integration (can be replaced with more sophisticated methods)
    const double mass = 1.0; // Unit mass
    const size_t n = m_state.positions.size();

    for (size_t i = 0; i < n; ++i) {
        // a = F / m
        m_state.accelerations[i] = forces[i] / mass;

        // v = v + a * dt
        m_state.velocities[i] += m_state.accelerations[i] * m_parameters.timeStep;

        // x = x + v * dt
        m_state.positions[i] += m_state.velocities[i] * m_parameters.timeStep;
    }
}

void SimulationEngine::checkConvergence()
{
    // Check if system has converged (optional)
    double totalEnergy = 0.0;

    for (size_t i = 0; i < m_state.positions.size(); ++i) {
        // Kinetic energy: 0.5 * m * v^2
        totalEnergy += 0.5 * m_state.velocities[i] * m_state.velocities[i];

        // Potential energy: 0.5 * k * x^2
        totalEnergy += 0.5 * m_parameters.stiffness * m_state.positions[i] * m_state.positions[i];
    }

    // Could check if energy is below threshold for convergence
    // For now, just continue until time limit
}

void SimulationEngine::finalizeSimulation()
{
    // Cleanup and final calculations
    QMutexLocker locker(&m_mutex);

    // Could save final state, compute statistics, etc.
}
