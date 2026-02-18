#ifndef SHAREDMEMORYSENDER_H
#define SHAREDMEMORYSENDER_H

#include <QObject>
#include <QSharedMemory>
#include <QString>
#include <QMutex>
#include <vector>

/**
 * @brief Shared memory sender for inter-process communication
 * 
 * This class provides functionality to send simulation data
 * to other processes via shared memory. It supports:
 * - Creating and managing shared memory segments
 * - Writing structured data
 * - Thread-safe operations
 * - Automatic cleanup
 */
class SharedMemorySender : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Data packet structure for shared memory
     */
    struct DataPacket
    {
        // Header
        int version;
        int dataSize;
        double timestamp;
        int frameNumber;
        
        // Simulation state
        double currentTime;
        int currentStep;
        
        // Geometry data (simplified)
        int numPoints;
        int numElements;
        
        // Reserved for future use
        char reserved[64];
        
        DataPacket()
            : version(1)
            , dataSize(0)
            , timestamp(0.0)
            , frameNumber(0)
            , currentTime(0.0)
            , currentStep(0)
            , numPoints(0)
            , numElements(0)
        {
            memset(reserved, 0, sizeof(reserved));
        }
    };

    explicit SharedMemorySender(QObject *parent = nullptr);
    ~SharedMemorySender();

    /**
     * @brief Initialize shared memory with a given key
     * @param key Shared memory key identifier
     * @param size Size of shared memory in bytes
     * @return true if successful
     */
    bool initialize(const QString& key, int size);

    /**
     * @brief Send a data packet to shared memory
     * @param packet The data packet to send
     * @return true if successful
     */
    bool sendPacket(const DataPacket& packet);

    /**
     * @brief Send raw data to shared memory
     * @param data Pointer to data
     * @param size Size of data in bytes
     * @return true if successful
     */
    bool sendData(const void* data, int size);

    /**
     * @brief Send vector data
     * @param data Vector of doubles
     * @return true if successful
     */
    bool sendVectorData(const std::vector<double>& data);

    /**
     * @brief Check if shared memory is initialized
     * @return true if initialized
     */
    bool isInitialized() const { return m_isInitialized; }

    /**
     * @brief Get the shared memory key
     * @return The key string
     */
    QString getKey() const { return m_key; }

    /**
     * @brief Get the shared memory size
     * @return Size in bytes
     */
    int getSize() const { return m_size; }

    /**
     * @brief Detach from shared memory
     */
    void detach();

    /**
     * @brief Get last error message
     * @return Error message string
     */
    QString getLastError() const { return m_lastError; }

signals:
    void dataSent(int size);
    void errorOccurred(const QString& error);

private:
    // Helper methods
    void setError(const QString& error);
    bool checkInitialized();

    // Data members
    QSharedMemory* m_sharedMemory;
    QString m_key;
    int m_size;
    bool m_isInitialized;
    QString m_lastError;
    mutable QMutex m_mutex;
    
    // Statistics
    int m_packetsSent;
    qint64 m_totalBytesSent;
};

#endif // SHAREDMEMORYSENDER_H
