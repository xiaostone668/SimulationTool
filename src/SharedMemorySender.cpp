#include "SharedMemorySender.h"
#include <QMutexLocker>
#include <cstring>

SharedMemorySender::SharedMemorySender(QObject *parent)
    : QObject(parent)
    , m_sharedMemory(nullptr)
    , m_size(0)
    , m_isInitialized(false)
    , m_packetsSent(0)
    , m_totalBytesSent(0)
{
}

SharedMemorySender::~SharedMemorySender()
{
    detach();
    if (m_sharedMemory) {
        delete m_sharedMemory;
        m_sharedMemory = nullptr;
    }
}

bool SharedMemorySender::initialize(const QString& key, int size)
{
    QMutexLocker locker(&m_mutex);

    // Clean up existing shared memory
    if (m_sharedMemory) {
        if (m_sharedMemory->isAttached()) {
            m_sharedMemory->detach();
        }
        delete m_sharedMemory;
        m_sharedMemory = nullptr;
    }

    m_key = key;
    m_size = size;

    // Create shared memory
    m_sharedMemory = new QSharedMemory(key);

    // Try to attach first (in case it already exists)
    if (m_sharedMemory->attach()) {
        // Already exists, detach and recreate
        m_sharedMemory->detach();
    }

    // Create new shared memory segment
    if (!m_sharedMemory->create(size)) {
        setError(QString("Failed to create shared memory: %1").arg(m_sharedMemory->errorString()));
        m_isInitialized = false;
        return false;
    }

    m_isInitialized = true;
    m_packetsSent = 0;
    m_totalBytesSent = 0;

    return true;
}

bool SharedMemorySender::sendPacket(const DataPacket& packet)
{
    return sendData(&packet, sizeof(DataPacket));
}

bool SharedMemorySender::sendData(const void* data, int size)
{
    if (!checkInitialized()) {
        return false;
    }

    QMutexLocker locker(&m_mutex);

    if (size > m_size) {
        setError(QString("Data size (%1) exceeds shared memory size (%2)").arg(size).arg(m_size));
        return false;
    }

    // Lock shared memory
    if (!m_sharedMemory->lock()) {
        setError(QString("Failed to lock shared memory: %1").arg(m_sharedMemory->errorString()));
        return false;
    }

    // Copy data to shared memory
    void* sharedData = m_sharedMemory->data();
    if (sharedData) {
        std::memcpy(sharedData, data, size);
    } else {
        m_sharedMemory->unlock();
        setError("Shared memory data pointer is null");
        return false;
    }

    // Unlock shared memory
    if (!m_sharedMemory->unlock()) {
        setError(QString("Failed to unlock shared memory: %1").arg(m_sharedMemory->errorString()));
        return false;
    }

    // Update statistics
    m_packetsSent++;
    m_totalBytesSent += size;

    emit dataSent(size);

    return true;
}

bool SharedMemorySender::sendVectorData(const std::vector<double>& data)
{
    if (!checkInitialized()) {
        return false;
    }

    int dataSize = static_cast<int>(data.size() * sizeof(double));

    // Create a packet with header
    DataPacket packet;
    packet.dataSize = dataSize;
    packet.numPoints = static_cast<int>(data.size());

    // First send the header
    if (!sendPacket(packet)) {
        return false;
    }

    // Then send the actual data if it fits
    if (dataSize + sizeof(DataPacket) <= m_size) {
        QMutexLocker locker(&m_mutex);

        if (!m_sharedMemory->lock()) {
            setError(QString("Failed to lock shared memory: %1").arg(m_sharedMemory->errorString()));
            return false;
        }

        char* sharedData = static_cast<char*>(m_sharedMemory->data());
        if (sharedData) {
            // Copy header
            std::memcpy(sharedData, &packet, sizeof(DataPacket));
            // Copy vector data after header
            std::memcpy(sharedData + sizeof(DataPacket), data.data(), dataSize);
        } else {
            m_sharedMemory->unlock();
            setError("Shared memory data pointer is null");
            return false;
        }

        m_sharedMemory->unlock();

        m_totalBytesSent += (sizeof(DataPacket) + dataSize);
        emit dataSent(sizeof(DataPacket) + dataSize);

        return true;
    } else {
        setError("Vector data too large for shared memory");
        return false;
    }
}

void SharedMemorySender::detach()
{
    QMutexLocker locker(&m_mutex);

    if (m_sharedMemory && m_sharedMemory->isAttached()) {
        m_sharedMemory->detach();
    }

    m_isInitialized = false;
}

void SharedMemorySender::setError(const QString& error)
{
    m_lastError = error;
    emit errorOccurred(error);
}

bool SharedMemorySender::checkInitialized()
{
    if (!m_isInitialized) {
        setError("Shared memory not initialized");
        return false;
    }
    return true;
}
