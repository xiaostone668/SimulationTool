#include "STEPReader.h"

// OpenCASCADE includes (common)
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <Quantity_Color.hxx>
#include <AIS_Shape.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#ifndef OCC_NO_STEP
#include <STEPControl_Reader.hxx>
#endif

STEPReader::STEPReader(QObject *parent)
    : QObject(parent)
    , m_shape()
    , m_aisShape(nullptr)
{
}

STEPReader::~STEPReader()
{
    clear();
}

bool STEPReader::loadSTEPFile(const QString& filePath)
{
    emit loadingStarted();

#ifdef OCC_NO_STEP
    (void)filePath;
    setError("STEP 功能未启用：当前 OCCT 未包含 STEP 库。请使用 scripts/build_occt.ps1 编译完整 OCCT 并设置 OCC_ROOT。");
    emit loadingFinished(false);
    return false;
#else
    try {
        STEPControl_Reader reader;

        IFSelect_ReturnStatus status = reader.ReadFile(filePath.toStdString().c_str());

        if (status != IFSelect_RetDone) {
            setError("Failed to read STEP file");
            emit loadingFinished(false);
            return false;
        }

        emit loadingProgress(30);

        Standard_Integer nbRoots = reader.NbRootsForTransfer();
        if (nbRoots == 0) {
            setError("No roots found in STEP file");
            emit loadingFinished(false);
            return false;
        }

        reader.TransferRoots();

        emit loadingProgress(60);

        m_shape = reader.OneShape();

        if (m_shape.IsNull()) {
            setError("Failed to extract shape from STEP file");
            emit loadingFinished(false);
            return false;
        }

        emit loadingProgress(80);

        analyzeShape();
        computeProperties();

        m_currentFilePath = filePath;

        emit loadingProgress(100);
        emit loadingFinished(true);

        return true;
    }
    catch (const Standard_Failure& e) {
        setError(QString("OpenCASCADE error: %1").arg(e.GetMessageString()));
        emit loadingFinished(false);
        return false;
    }
    catch (const std::exception& e) {
        setError(QString("Error: %1").arg(e.what()));
        emit loadingFinished(false);
        return false;
    }
    catch (...) {
        setError("Unknown error occurred while loading STEP file");
        emit loadingFinished(false);
        return false;
    }
#endif
}

void STEPReader::displayShape(const Handle(AIS_InteractiveContext)& context, bool fitAll)
{
    if (m_shape.IsNull() || context.IsNull()) {
        return;
    }

    // Remove previous shape if exists
    if (!m_aisShape.IsNull()) {
        context->Remove(m_aisShape, Standard_False);
        m_aisShape.Nullify();
    }

    // Create AIS shape
    m_aisShape = new AIS_Shape(m_shape);

    // Set display properties
    m_aisShape->SetColor(Quantity_NOC_YELLOW);
    m_aisShape->SetDisplayMode(AIS_Shaded);

    // Display shape: use Standard_False to avoid immediate redraw
    // (caller is responsible for calling FitAll + Redraw on the view)
    context->Display(m_aisShape, Standard_False);

    if (fitAll) {
        // Safely iterate active views for FitAll
        Handle(V3d_Viewer) viewer = context->CurrentViewer();
        if (!viewer.IsNull()) {
            viewer->InitActiveViews();
            if (viewer->MoreActiveViews()) {
                Handle(V3d_View) view = viewer->ActiveView();
                if (!view.IsNull()) {
                    view->FitAll();
                    view->Redraw();
                }
            }
        }
    }
}

STEPReader::GeometryInfo STEPReader::getGeometryInfo() const
{
    return m_geometryInfo;
}

void STEPReader::clear()
{
    m_shape.Nullify();
    if (!m_aisShape.IsNull()) {
        m_aisShape.Nullify();
    }
    m_geometryInfo = GeometryInfo();
    m_currentFilePath.clear();
    m_lastError.clear();
}

void STEPReader::analyzeShape()
{
    if (m_shape.IsNull()) {
        return;
    }

    m_geometryInfo = GeometryInfo();

    // Count solids
    for (TopExp_Explorer exp(m_shape, TopAbs_SOLID); exp.More(); exp.Next()) {
        m_geometryInfo.numSolids++;
    }

    // Count shells
    for (TopExp_Explorer exp(m_shape, TopAbs_SHELL); exp.More(); exp.Next()) {
        m_geometryInfo.numShells++;
    }

    // Count faces
    for (TopExp_Explorer exp(m_shape, TopAbs_FACE); exp.More(); exp.Next()) {
        m_geometryInfo.numFaces++;
    }

    // Count edges
    for (TopExp_Explorer exp(m_shape, TopAbs_EDGE); exp.More(); exp.Next()) {
        m_geometryInfo.numEdges++;
    }

    // Count vertices
    for (TopExp_Explorer exp(m_shape, TopAbs_VERTEX); exp.More(); exp.Next()) {
        m_geometryInfo.numVertices++;
    }
}

void STEPReader::computeProperties()
{
    if (m_shape.IsNull()) {
        return;
    }

    try {
        // Compute volume
        GProp_GProps volumeProps;
        BRepGProp::VolumeProperties(m_shape, volumeProps);
        m_geometryInfo.volume = volumeProps.Mass();

        // Compute surface area
        GProp_GProps surfaceProps;
        BRepGProp::SurfaceProperties(m_shape, surfaceProps);
        m_geometryInfo.surfaceArea = surfaceProps.Mass();

        // Compute bounding box
        Bnd_Box boundingBox;
        BRepBndLib::Add(m_shape, boundingBox);

        if (!boundingBox.IsVoid()) {
            Standard_Real xMin, yMin, zMin, xMax, yMax, zMax;
            boundingBox.Get(xMin, yMin, zMin, xMax, yMax, zMax);

            double dx = xMax - xMin;
            double dy = yMax - yMin;
            double dz = zMax - zMin;

            m_geometryInfo.boundingBoxVolume = dx * dy * dz;
        }
    }
    catch (...) {
        // If property computation fails, just continue with zero values
    }
}

void STEPReader::setError(const QString& error)
{
    m_lastError = error;
    emit errorOccurred(error);
}
