#include <iostream>
#include <string>
#include <fstream>

#include <TopoDS_Shape.hxx>
#include <TopExp_Explorer.hxx>
#include <TopExp.hxx>
#include <TopTools_IndexedMapOfShape.hxx>
#include <TopAbs.hxx>
#include <STEPControl_Reader.hxx>
#include <IFSelect_ReturnStatus.hxx>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <step_file_path>" << std::endl;
        return 1;
    }
    
    std::string filePath = argv[1];
    std::cout << "Loading STEP file: " << filePath << std::endl;
    
    try {
        STEPControl_Reader reader;
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());
        
        if (status != IFSelect_RetDone) {
            std::cout << "ERROR: Failed to read STEP file" << std::endl;
            return 1;
        }
        
        Standard_Integer nbRoots = reader.NbRootsForTransfer();
        std::cout << "Number of roots: " << nbRoots << std::endl;
        
        if (nbRoots == 0) {
            std::cout << "ERROR: No roots found in STEP file" << std::endl;
            return 1;
        }
        
        reader.TransferRoots();
        TopoDS_Shape shape = reader.OneShape();
        
        if (shape.IsNull()) {
            std::cout << "ERROR: Failed to extract shape from STEP file" << std::endl;
            return 1;
        }
        
        // Analyze shape
        TopTools_IndexedMapOfShape solidMap, shellMap, faceMap, edgeMap, vertexMap;
        
        TopExp::MapShapes(shape, TopAbs_SOLID, solidMap);
        TopExp::MapShapes(shape, TopAbs_SHELL, shellMap);
        TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
        TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);
        TopExp::MapShapes(shape, TopAbs_VERTEX, vertexMap);
        
        std::cout << "\n=== Geometry Analysis ===" << std::endl;
        std::cout << "Solids: " << solidMap.Extent() << std::endl;
        std::cout << "Shells: " << shellMap.Extent() << std::endl;
        std::cout << "Faces: " << faceMap.Extent() << std::endl;
        std::cout << "Edges: " << edgeMap.Extent() << std::endl;
        std::cout << "Vertices: " << vertexMap.Extent() << std::endl;
        
        // Check shape type
        TopAbs_ShapeEnum shapeType = shape.ShapeType();
        std::cout << "\nRoot shape type: " << shapeType << std::endl;
        
        // If it's a compound, explore sub-shapes
        if (shapeType == TopAbs_COMPOUND || shapeType == TopAbs_COMPSOLID) {
            std::cout << "\nExploring compound structure..." << std::endl;
            
            int shellIdx = 0;
            TopExp_Explorer shellExp(shape, TopAbs_SHELL);
            for (; shellExp.More(); shellExp.Next()) {
                shellIdx++;
                const TopoDS_Shape& shell = shellExp.Current();
                std::cout << "\nShell " << shellIdx << " (type: " << shell.ShapeType() << ")" << std::endl;
                
                // Count faces in this shell
                TopTools_IndexedMapOfShape shellFaceMap;
                TopExp::MapShapes(shell, TopAbs_FACE, shellFaceMap);
                std::cout << "  Faces in shell: " << shellFaceMap.Extent() << std::endl;
                
                // Count edges in this shell
                TopTools_IndexedMapOfShape shellEdgeMap;
                TopExp::MapShapes(shell, TopAbs_EDGE, shellEdgeMap);
                std::cout << "  Edges in shell: " << shellEdgeMap.Extent() << std::endl;
                
                // Explore faces
                int faceIdx = 0;
                TopExp_Explorer faceExp(shell, TopAbs_FACE);
                for (; faceExp.More(); faceExp.Next()) {
                    faceIdx++;
                    const TopoDS_Shape& face = faceExp.Current();
                    
                    // Count edges in this face
                    TopTools_IndexedMapOfShape faceEdgeMap;
                    TopExp::MapShapes(face, TopAbs_EDGE, faceEdgeMap);
                    std::cout << "  Face " << faceIdx << ": " << faceEdgeMap.Extent() << " edges" << std::endl;
                }
            }
        }
        
        // Try alternative counting methods
        std::cout << "\n=== Alternative Counting Methods ===" << std::endl;
        
        // Method 1: Using TopExp_Explorer
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
        
        std::cout << "TopExp_Explorer counts:" << std::endl;
        std::cout << "  Solids: " << solidCount << std::endl;
        std::cout << "  Shells: " << shellCount << std::endl;
        std::cout << "  Faces: " << faceCount << std::endl;
        std::cout << "  Edges: " << edgeCount << std::endl;
        std::cout << "  Vertices: " << vertexCount << std::endl;
        
        // Method 2: Recursive extraction from shells if needed
        if (shellCount > 0 && faceCount == 0) {
            std::cout << "\nWARNING: Shells found but no faces! Attempting recursive extraction..." << std::endl;
            
            // Re-count faces and edges by exploring each shell
            faceCount = 0;
            edgeCount = 0;
            vertexCount = 0;
            
            TopExp_Explorer shellExp2(shape, TopAbs_SHELL);
            for (; shellExp2.More(); shellExp2.Next()) {
                const TopoDS_Shape& shell = shellExp2.Current();
                
                TopExp_Explorer faceExp2(shell, TopAbs_FACE);
                for (; faceExp2.More(); faceExp2.Next()) faceCount++;
                
                TopExp_Explorer edgeExp2(shell, TopAbs_EDGE);
                for (; edgeExp2.More(); edgeExp2.Next()) edgeCount++;
                
                TopExp_Explorer vertexExp2(shell, TopAbs_VERTEX);
                for (; vertexExp2.More(); vertexExp2.Next()) vertexCount++;
            }
            
            std::cout << "After recursive extraction:" << std::endl;
            std::cout << "  Faces: " << faceCount << std::endl;
            std::cout << "  Edges: " << edgeCount << std::endl;
            std::cout << "  Vertices: " << vertexCount << std::endl;
        }
        
        std::cout << "\n=== Test Complete ===" << std::endl;
        
    } catch (const Standard_Failure& e) {
        std::cout << "OpenCASCADE error: " << e.GetMessageString() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "Unknown error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}