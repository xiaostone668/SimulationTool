#ifndef STEPREADER_H
#define STEPREADER_H

#include <QString>
#include <QObject>
#include <vector>

// OpenCASCADE includes
#include <TopoDS_Shape.hxx>
#include <TopoDS_Compound.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>

/**
 * @brief STEP file reader and geometry handler
 * 
 * This class encapsulates OpenCASCADE STEP file reading functionality
 * and provides methods to:
 * - Load STEP files
 * - Extract geometry information
 * - Display shapes in AIS context
 * - Query geometric properties
 */
class STEPReader : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief Geometric properties structure
     */
    struct GeometryInfo
    {
        int numSolids;
        int numShells;
        int numFaces;
        int numEdges;
        int numVertices;
        double volume;
        double surfaceArea;
        double boundingBoxVolume;
        
        GeometryInfo()
            : numSolids(0)
            , numShells(0)
            , numFaces(0)
            , numEdges(0)
            , numVertices(0)
            , volume(0.0)
            , surfaceArea(0.0)
            , boundingBoxVolume(0.0)
        {}
    };

    explicit STEPReader(QObject *parent = nullptr);
    ~STEPReader();

    /**
     * @brief Load a STEP file
     * @param filePath Path to the STEP file
     * @return true if successful, false otherwise
     */
    bool loadSTEPFile(const QString& filePath);

    /**
     * @brief Get the loaded shape
     * @return The TopoDS_Shape object
     */
    TopoDS_Shape getShape() const { return m_shape; }

    /**
     * @brief Display the shape in the given AIS context
     * @param context The AIS interactive context
     * @param fitAll Whether to fit all objects in view
     */
    void displayShape(const Handle(AIS_InteractiveContext)& context, bool fitAll = true);

    /**
     * @brief Get geometry information
     * @return GeometryInfo structure with shape statistics
     */
    GeometryInfo getGeometryInfo() const;

    /**
     * @brief Check if a shape is loaded
     * @return true if a shape is loaded
     */
    bool hasShape() const { return !m_shape.IsNull(); }

    /**
     * @brief Clear the current shape
     */
    void clear();

    /**
     * @brief Get the last error message
     * @return Error message string
     */
    QString getLastError() const { return m_lastError; }

signals:
    void loadingStarted();
    void loadingProgress(int progress);
    void loadingFinished(bool success);
    void errorOccurred(const QString& error);

private:
    // Helper methods
    void analyzeShape();
    void computeProperties();
    void setError(const QString& error);

    // Data members
    TopoDS_Shape m_shape;
    Handle(AIS_Shape) m_aisShape;
    GeometryInfo m_geometryInfo;
    QString m_lastError;
    QString m_currentFilePath;
};

#endif // STEPREADER_H
