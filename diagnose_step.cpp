#include <iostream>
#include <string>
#include <fstream>

// OpenCASCADE includes
#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopAbs.hxx>
#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>
#include <Standard_Failure.hxx>
#include <TCollection_AsciiString.hxx>

int main(int argc, char* argv[]) {
    std::string filePath = "examples/two_sheetbodies.stp";
    
    std::cout << "诊断STEP文件: " << filePath << std::endl;
    
    try {
        STEPControl_Reader reader;
        
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());
        
        std::cout << "ReadFile状态: " << status << " (0=RetDone)" << std::endl;
        
        if (status != IFSelect_RetDone) {
            std::cout << "错误: 无法读取STEP文件" << std::endl;
            return 1;
        }
        
        Standard_Integer nbRoots = reader.NbRootsForTransfer();
        std::cout << "传输根数量: " << nbRoots << std::endl;
        
        // 列出所有根实体
        for (int i = 1; i <= nbRoots; i++) {
            std::cout << "根实体 #" << i << ":" << std::endl;
            // 获取根实体类型
            Handle(Standard_Transient) root = reader.RootForTransfer(i);
            std::cout << "  类型: " << root->DynamicType()->Name() << std::endl;
        }
        
        if (nbRoots == 0) {
            std::cout << "错误: STEP文件中未找到根实体" << std::endl;
            return 1;
        }
        
        std::cout << "传输根实体..." << std::endl;
        reader.TransferRoots();
        
        // 获取所有形状，不仅仅是OneShape
        int numShapes = reader.NbShapes();
        std::cout << "总形状数量: " << numShapes << std::endl;
        
        // 列出所有形状
        for (int i = 1; i <= numShapes; i++) {
            TopoDS_Shape shape = reader.Shape(i);
            if (shape.IsNull()) {
                std::cout << "形状 #" << i << ": NULL" << std::endl;
            } else {
                TopAbs_ShapeEnum shapeType = shape.ShapeType();
                std::cout << "形状 #" << i << ": 类型=" << shapeType;
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
        
        // 使用OneShape获取主形状
        TopoDS_Shape shape = reader.OneShape();
        
        if (shape.IsNull()) {
            std::cout << "错误: 无法从STEP文件提取形状" << std::endl;
            return 1;
        }
        
        std::cout << "\n=== OneShape分析 ===" << std::endl;
        
        // 获取形状类型
        TopAbs_ShapeEnum shapeType = shape.ShapeType();
        std::cout << "主形状类型: ";
        switch (shapeType) {
            case TopAbs_COMPOUND: std::cout << "COMPOUND"; break;
            case TopAbs_COMPSOLID: std::cout << "COMPSOLID"; break;
            case TopAbs_SOLID: std::cout << "SOLID"; break;
            case TopAbs_SHELL: std::cout << "SHELL"; break;
            case TopAbs_FACE: std::cout << "FACE"; break;
            case TopAbs_WIRE: std::cout << "WIRE"; break;
            case TopAbs_EDGE: std::cout << "EDGE"; break;
            case TopAbs_VERTEX: std::cout << "VERTEX"; break;
            case TopAbs_SHAPE: std::cout << "SHAPE"; break;
            default: std::cout << "UNKNOWN (" << shapeType << ")"; break;
        }
        std::cout << std::endl;
        
        // 使用TopExp_Explorer统计
        std::cout << "\n使用TopExp_Explorer统计:" << std::endl;
        
        int solidCount = 0, shellCount = 0, faceCount = 0, edgeCount = 0, vertexCount = 0;
        
        TopExp_Explorer solidExp(shape, TopAbs_SOLID);
        for (; solidExp.More(); solidExp.Next()) solidCount++;
        
        TopExp_Explorer shellExp(shape, TopAbs_SHELL);
        for (; shellExp.More(); shellExp.Next()) shellCount++;
        
        TopExp_Explorer faceExp(shape, TopAbs_FACE);
        for (; faceExp.More(); faceExp.Next()) faceCount++;
        
        TopExp_Explorer edgeExp(shape, TopAbs_EDGE);
        for (; edgeExp.More(); edgeExp.Next()) edgeCount++;
        
        TopExp_Explorer vertexExp(shape, TopAbs_VERTEX);
        for (; vertexExp.More(); vertexExp.Next()) vertexCount++;
        
        std::cout << "  实体: " << solidCount << std::endl;
        std::cout << "  壳体: " << shellCount << std::endl;
        std::cout << "  面: " << faceCount << std::endl;
        std::cout << "  边: " << edgeCount << std::endl;
        std::cout << "  顶点: " << vertexCount << std::endl;
        
        // 使用TopExp::MapShapes统计
        std::cout << "\n使用TopExp::MapShapes统计:" << std::endl;
        
        TopTools_IndexedMapOfShape solidMap, shellMap, faceMap, edgeMap, vertexMap;
        
        TopExp::MapShapes(shape, TopAbs_SOLID, solidMap);
        TopExp::MapShapes(shape, TopAbs_SHELL, shellMap);
        TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
        TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);
        TopExp::MapShapes(shape, TopAbs_VERTEX, vertexMap);
        
        std::cout << "  实体: " << solidMap.Extent() << std::endl;
        std::cout << "  壳体: " << shellMap.Extent() << std::endl;
        std::cout << "  面: " << faceMap.Extent() << std::endl;
        std::cout << "  边: " << edgeMap.Extent() << std::endl;
        std::cout << "  顶点: " << vertexMap.Extent() << std::endl;
        
        // 如果是复合体，深入分析
        if (shapeType == TopAbs_COMPOUND || shapeType == TopAbs_COMPSOLID) {
            std::cout << "\n=== 复合体深入分析 ===" << std::endl;
            
            // 统计所有面（包括嵌套的）
            int totalFaces = 0;
            TopExp_Explorer faceExpAll(shape, TopAbs_FACE, TopAbs_SHAPE);
            for (; faceExpAll.More(); faceExpAll.Next()) totalFaces++;
            std::cout << "所有面（包括嵌套）: " << totalFaces << std::endl;
            
            // 统计所有边
            int totalEdges = 0;
            TopExp_Explorer edgeExpAll(shape, TopAbs_EDGE, TopAbs_SHAPE);
            for (; edgeExpAll.More(); edgeExpAll.Next()) totalEdges++;
            std::cout << "所有边（包括嵌套）: " << totalEdges << std::endl;
            
            // 详细列出所有壳体和面
            int shellIdx = 0;
            TopExp_Explorer shellExp2(shape, TopAbs_SHELL);
            for (; shellExp2.More(); shellExp2.Next()) {
                shellIdx++;
                const TopoDS_Shape& shell = shellExp2.Current();
                
                std::cout << "\n壳体 " << shellIdx << ":" << std::endl;
                
                // 壳体中面的数量
                int facesInShell = 0;
                TopExp_Explorer faceExp2(shell, TopAbs_FACE);
                for (; faceExp2.More(); faceExp2.Next()) facesInShell++;
                std::cout << "  包含面: " << facesInShell << std::endl;
                
                // 列出每个面
                if (facesInShell > 0) {
                    int faceIdx = 0;
                    TopExp_Explorer faceExp3(shell, TopAbs_FACE);
                    for (; faceExp3.More(); faceExp3.Next()) {
                        faceIdx++;
                        const TopoDS_Shape& face = faceExp3.Current();
                        
                        // 面中的边数量
                        int edgesInFace = 0;
                        TopExp_Explorer edgeExp3(face, TopAbs_EDGE);
                        for (; edgeExp3.More(); edgeExp3.Next()) edgesInFace++;
                        
                        std::cout << "  面 " << faceIdx << ": " << edgesInFace << " 条边" << std::endl;
                    }
                }
            }
        }
        
        std::cout << "\n=== 诊断完成 ===" << std::endl;
        
    } catch (const Standard_Failure& e) {
        std::cout << "OpenCASCADE错误: " << e.GetMessageString() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cout << "标准错误: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "未知错误" << std::endl;
        return 1;
    }
    
    return 0;
}