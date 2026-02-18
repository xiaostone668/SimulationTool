#include "STEPReader.h"

// OpenCASCADE includes (common)
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopAbs.hxx>
#include <GProp_GProps.hxx>
#include <BRepGProp.hxx>
#include <Bnd_Box.hxx>
#include <BRepBndLib.hxx>
#include <Quantity_Color.hxx>
#include <AIS_Shape.hxx>
#include <V3d_Viewer.hxx>
#include <V3d_View.hxx>
#include <BRep_Builder.hxx>  // For BRep_Builder and TopoDS_Compound
#include <fstream>
#include <iostream>
#ifndef OCC_NO_STEP
#include <STEPControl_Reader.hxx>
#include <Interface_Static.hxx>
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

        std::cout << "[STEPReader] Loading STEP file: " << filePath.toStdString() << std::endl;
        
        // 设置 STEP 控制器参数以支持 AP214 schema
        // 这些设置帮助 OpenCASCADE 更好地处理 automotive design schema
        try {
            // 设置 schema 为 AP214 (AUTOMOTIVE_DESIGN)
            Interface_Static::SetCVal("xstep.cascade.unit", "M");
            Interface_Static::SetCVal("read.step.schema", "AP214");
            Interface_Static::SetIVal("read.step.product.mode", 1);
            Interface_Static::SetIVal("read.step.shape.repr", 1);
            Interface_Static::SetIVal("read.step.assembly.level", 2);
            Interface_Static::SetIVal("read.step.product.context", 1);
            Interface_Static::SetIVal("read.step.shape.relationship", 1);
        } catch (...) {
            std::cout << "[STEPReader] Note: Could not set all STEP controller parameters" << std::endl;
        }

        std::cout << "[STEPReader] STEP controller parameters configured" << std::endl;
        
        // 设置更多容错参数
        try {
            Interface_Static::SetIVal("read.step.ambiguity", 1); // 允许歧义
            Interface_Static::SetIVal("read.step.product.mode", 1);
            Interface_Static::SetIVal("read.step.shape.repr", 1);
            Interface_Static::SetIVal("read.step.assembly.level", 2);
            Interface_Static::SetIVal("read.step.product.context", 1);
            Interface_Static::SetIVal("read.step.shape.relationship", 1);
            Interface_Static::SetCVal("read.step.nonmanifold", "on"); // 允许非流形几何
            Interface_Static::SetCVal("read.step.surfacecurve.mode", "3d"); // 曲面曲线模式
        } catch (...) {
            std::cout << "[STEPReader] Note: Could not set all advanced STEP parameters" << std::endl;
        }
        
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.toStdString().c_str());

        std::cout << "[STEPReader] ReadFile status: " << status << " (0=RetDone)" << std::endl;
        
        // 输出更多诊断信息
        if (status == IFSelect_RetError || status == IFSelect_RetFail) {
            // 尝试获取更详细的错误信息
            std::cout << "[STEPReader] ReadFile failed with status: " << status << std::endl;
            
            // 检查文件是否存在
            std::ifstream testFile(filePath.toStdString());
            if (!testFile.is_open()) {
                setError(QString("File does not exist or cannot be opened: %1").arg(filePath));
                emit loadingFinished(false);
                return false;
            }
            testFile.close();
            
            setError("Failed to read STEP file (OpenCASCADE parser error)");
            emit loadingFinished(false);
            return false;
        }

        // 即使有警告也继续处理
        std::cout << "[STEPReader] STEP file read with status: " << status << " (continuing anyway)" << std::endl;
        
        Standard_Integer nbRoots = reader.NbRootsForTransfer();
        std::cout << "[STEPReader] Number of roots for transfer: " << nbRoots << std::endl;
        
        // 列出所有根实体
        for (int i = 1; i <= nbRoots; i++) {
            Handle(Standard_Transient) root = reader.RootForTransfer(i);
            if (!root.IsNull()) {
                std::cout << "[STEPReader] Root #" << i << " type: " << root->DynamicType()->Name() << std::endl;
            }
        }

        if (nbRoots == 0) {
            setError("No roots found in STEP file");
            emit loadingFinished(false);
            return false;
        }

        emit loadingProgress(30);

        std::cout << "[STEPReader] Transferring roots..." << std::endl;
        
        // 尝试传输每个根实体并检查结果
        int numRoots = reader.NbRootsForTransfer();
        int numShapesTransferred = 0;
        
        for (int i = 1; i <= numRoots; i++) {
            std::cout << "[STEPReader] Transferring root #" << i << "..." << std::endl;
            bool transferResult = reader.TransferRoot(i);
            std::cout << "[STEPReader] Root #" << i << " transfer result: " << (transferResult ? "success" : "failed") << std::endl;
            
            if (transferResult) {
                // 检查这个根实体传输后产生的形状
                int shapeCount = reader.NbShapes();
                std::cout << "[STEPReader] After transferring root #" << i << ", shape count: " << shapeCount << std::endl;
                
                for (int j = 1; j <= shapeCount; j++) {
                    TopoDS_Shape shape = reader.Shape(j);
                    if (!shape.IsNull()) {
                        numShapesTransferred++;
                        TopAbs_ShapeEnum shapeType = shape.ShapeType();
                        std::cout << "[STEPReader] Shape #" << j << " from root #" << i << ": type=" << shapeType;
                        switch (shapeType) {
                            case TopAbs_COMPOUND: std::cout << " (COMPOUND)"; break;
                            case TopAbs_COMPSOLID: std::cout << " (COMPSOLID)"; break;
                            case TopAbs_SOLID: std::cout << " (SOLID)"; break;
                            case TopAbs_SHELL: std::cout << " (SHELL)"; break;
                            case TopAbs_FACE: std::cout << " (FACE)"; break;
                            case TopAbs_WIRE: std::cout << " (WIRE)"; break;
                            case TopAbs_EDGE: std::cout << " (EDGE)"; break;
                            case TopAbs_VERTEX: std::cout << " (VERTEX)"; break;
                            case TopAbs_SHAPE: std::cout << " (SHAPE)"; break;
                        }
                        std::cout << std::endl;
                    }
                }
            }
        }
        
        // 也尝试传统的TransferRoots方法
        if (numShapesTransferred == 0) {
            std::cout << "[STEPReader] No shapes from individual transfer, trying TransferRoots..." << std::endl;
            reader.TransferRoots();
        }

        // 输出所有形状信息
        int numShapes = reader.NbShapes();
        std::cout << "[STEPReader] Total shapes in reader: " << numShapes << std::endl;
        
        if (numShapes > 0) {
            for (int i = 1; i <= numShapes; i++) {
                TopoDS_Shape shape = reader.Shape(i);
                if (shape.IsNull()) {
                    std::cout << "[STEPReader] Shape #" << i << ": NULL" << std::endl;
                } else {
                    TopAbs_ShapeEnum shapeType = shape.ShapeType();
                    std::cout << "[STEPReader] Shape #" << i << ": type=" << shapeType;
                    switch (shapeType) {
                        case TopAbs_COMPOUND: std::cout << " (COMPOUND)"; break;
                        case TopAbs_COMPSOLID: std::cout << " (COMPSOLID)"; break;
                        case TopAbs_SOLID: std::cout << " (SOLID)"; break;
                        case TopAbs_SHELL: std::cout << " (SHELL)"; break;
                        case TopAbs_FACE: std::cout << " (FACE)"; break;
                        case TopAbs_WIRE: std::cout << " (WIRE)"; break;
                        case TopAbs_EDGE: std::cout << " (EDGE)"; break;
                        case TopAbs_VERTEX: std::cout << " (VERTEX)"; break;
                        case TopAbs_SHAPE: std::cout << " (SHAPE)"; break;
                    }
                    std::cout << std::endl;
                }
            }
        } else {
            std::cout << "[STEPReader] WARNING: No shapes found in reader after transfer!" << std::endl;
            
            // 尝试检查是否有其他类型的几何数据
            std::cout << "[STEPReader] Checking for alternative geometry representations..." << std::endl;
            
            // 这里可以添加其他诊断，比如检查是否有BREP数据
        }

        emit loadingProgress(60);

        m_shape = reader.OneShape();

        std::cout << "[STEPReader] OneShape extracted. IsNull: " << m_shape.IsNull() << std::endl;
        if (!m_shape.IsNull()) {
            TopAbs_ShapeEnum shapeType = m_shape.ShapeType();
            std::cout << "[STEPReader] OneShape type: " << shapeType << std::endl;
        }

        // 如果OneShape失败，但存在其他形状，尝试手动组合它们
        if (m_shape.IsNull() && numShapes > 0) {
            std::cout << "[STEPReader] OneShape failed but " << numShapes << " shapes exist. Attempting to combine..." << std::endl;
            
            BRep_Builder builder;
            TopoDS_Compound compound;
            builder.MakeCompound(compound);
            
            int combinedCount = 0;
            for (int i = 1; i <= numShapes; i++) {
                TopoDS_Shape shape = reader.Shape(i);
                if (!shape.IsNull()) {
                    builder.Add(compound, shape);
                    combinedCount++;
                }
            }
            
            if (combinedCount > 0) {
                m_shape = compound;
                std::cout << "[STEPReader] Successfully combined " << combinedCount << " shapes into a compound" << std::endl;
            } else {
                std::cout << "[STEPReader] Could not combine any shapes" << std::endl;
            }
        }

        if (m_shape.IsNull()) {
            setError("Failed to extract shape from STEP file (no geometry found)");
            emit loadingFinished(false);
            return false;
        }

        emit loadingProgress(80);

        std::cout << "[STEPReader] Analyzing shape..." << std::endl;
        analyzeShape();
        computeProperties();

        m_currentFilePath = filePath;

        emit loadingProgress(100);
        emit loadingFinished(true);

        std::cout << "[STEPReader] STEP file loaded successfully" << std::endl;
        return true;
    }
    catch (const Standard_Failure& e) {
        std::cout << "[STEPReader] OpenCASCADE error: " << e.GetMessageString() << std::endl;
        setError(QString("OpenCASCADE error: %1").arg(e.GetMessageString()));
        emit loadingFinished(false);
        return false;
    }
    catch (const std::exception& e) {
        std::cout << "[STEPReader] Standard error: " << e.what() << std::endl;
        setError(QString("Error: %1").arg(e.what()));
        emit loadingFinished(false);
        return false;
    }
    catch (...) {
        std::cout << "[STEPReader] Unknown error occurred" << std::endl;
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

    // 使用TopExp::MapShapes进行准确的几何统计
    // 这是OpenCASCADE中最可靠的统计方法
    
    TopTools_IndexedMapOfShape solidMap, shellMap, faceMap, edgeMap, vertexMap;
    
    // 统计所有类型的几何元素
    TopExp::MapShapes(m_shape, TopAbs_SOLID, solidMap);
    TopExp::MapShapes(m_shape, TopAbs_SHELL, shellMap);
    TopExp::MapShapes(m_shape, TopAbs_FACE, faceMap);
    TopExp::MapShapes(m_shape, TopAbs_EDGE, edgeMap);
    TopExp::MapShapes(m_shape, TopAbs_VERTEX, vertexMap);
    
    m_geometryInfo.numSolids = solidMap.Extent();
    m_geometryInfo.numShells = shellMap.Extent();
    m_geometryInfo.numFaces = faceMap.Extent();
    m_geometryInfo.numEdges = edgeMap.Extent();
    m_geometryInfo.numVertices = vertexMap.Extent();
    
    // 调试输出
    std::cout << "[DEBUG] analyzeShape (TopExp::MapShapes): " 
              << "Solids=" << m_geometryInfo.numSolids
              << ", Shells=" << m_geometryInfo.numShells
              << ", Faces=" << m_geometryInfo.numFaces
              << ", Edges=" << m_geometryInfo.numEdges
              << ", Vertices=" << m_geometryInfo.numVertices
              << std::endl;
    
    // 如果面或边的数量为0，尝试使用递归探索
    if (m_geometryInfo.numFaces == 0 || m_geometryInfo.numEdges == 0) {
        std::cout << "[DEBUG] Low geometry count detected. Trying recursive exploration..." << std::endl;
        
        // 使用递归的TopExp_Explorer进行深度探索
        int faceCount = 0;
        int edgeCount = 0;
        int vertexCount = 0;
        
        // 递归探索所有面
        TopExp_Explorer faceExp(m_shape, TopAbs_FACE, TopAbs_SHAPE);
        for (; faceExp.More(); faceExp.Next()) {
            faceCount++;
            
            // 探索这个面中的边
            const TopoDS_Shape& face = faceExp.Current();
            TopExp_Explorer edgeExp(face, TopAbs_EDGE);
            for (; edgeExp.More(); edgeExp.Next()) {
                edgeCount++;
                
                // 探索这个边中的顶点
                const TopoDS_Shape& edge = edgeExp.Current();
                TopExp_Explorer vertexExp(edge, TopAbs_VERTEX);
                for (; vertexExp.More(); vertexExp.Next()) {
                    vertexCount++;
                }
            }
        }
        
        // 如果通过递归探索找到了更多几何元素，更新计数
        if (faceCount > m_geometryInfo.numFaces) {
            m_geometryInfo.numFaces = faceCount;
        }
        if (edgeCount > m_geometryInfo.numEdges) {
            m_geometryInfo.numEdges = edgeCount;
        }
        if (vertexCount > m_geometryInfo.numVertices) {
            m_geometryInfo.numVertices = vertexCount;
        }
        
        std::cout << "[DEBUG] After recursive exploration: "
                  << "Faces=" << m_geometryInfo.numFaces
                  << ", Edges=" << m_geometryInfo.numEdges
                  << ", Vertices=" << m_geometryInfo.numVertices
                  << std::endl;
    }
    
    // 检查形状类型
    TopAbs_ShapeEnum shapeType = m_shape.ShapeType();
    std::cout << "[DEBUG] Root shape type: " << shapeType << std::endl;
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
