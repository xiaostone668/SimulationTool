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
    
    std::cout << "Testing STEP file: " << filePath << std::endl;
    std::cout << "File size: ";
    
    // Check file exists
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << "ERROR: File does not exist or cannot be opened!" << std::endl;
        return 1;
    }
    
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.close();
    
    std::cout << fileSize << " bytes" << std::endl;
    
    if (fileSize == 0) {
        std::cout << "ERROR: File is empty!" << std::endl;
        return 1;
    }
    
    // Read first few lines to check if it's a valid STEP file
    std::cout << "\nChecking file header..." << std::endl;
    std::ifstream file2(filePath);
    std::string line;
    int lineCount = 0;
    bool hasSTEP = false;
    
    while (std::getline(file2, line) && lineCount < 10) {
        lineCount++;
        std::cout << "Line " << lineCount << ": " << line.substr(0, 50) << (line.length() > 50 ? "..." : "") << std::endl;
        
        // Check for STEP file signature
        if (line.find("ISO-10303-21") != std::string::npos || 
            line.find("STEP;") != std::string::npos ||
            line.find("HEADER;") != std::string::npos) {
            hasSTEP = true;
        }
    }
    file2.close();
    
    if (!hasSTEP) {
        std::cout << "WARNING: File may not be a valid STEP file (missing STEP signature)" << std::endl;
    } else {
        std::cout << "File appears to be a valid STEP file" << std::endl;
    }
    
    try {
        std::cout << "\n=== Loading STEP file with OpenCASCADE ===" << std::endl;
        
        STEPControl_Reader reader;
        
        IFSelect_ReturnStatus status = reader.ReadFile(filePath.c_str());
        
        std::cout << "ReadFile status: " << status << std::endl;
        
        if (status != IFSelect_RetDone) {
            std::cout << "ERROR: Failed to read STEP file (status != IFSelect_RetDone)" << std::endl;
            std::cout << "Expected: " << IFSelect_RetDone << " (IFSelect_RetDone)" << std::endl;
            return 1;
        }
        
        std::cout << "STEP file read successfully!" << std::endl;
        
        Standard_Integer nbRoots = reader.NbRootsForTransfer();
        std::cout << "Number of roots for transfer: " << nbRoots << std::endl;
        
        if (nbRoots == 0) {
            std::cout << "ERROR: No roots found in STEP file" << std::endl;
            return 1;
        }
        
        std::cout << "Transferring roots..." << std::endl;
        reader.TransferRoots();
        
        TopoDS_Shape shape = reader.OneShape();
        
        if (shape.IsNull()) {
            std::cout << "ERROR: Failed to extract shape from STEP file (shape is null)" << std::endl;
            return 1;
        }
        
        std::cout << "Shape extracted successfully!" << std::endl;
        
        // Analyze the shape in detail
        std::cout << "\n=== Detailed Shape Analysis ===" << std::endl;
        
        // Get shape type
        TopAbs_ShapeEnum shapeType = shape.ShapeType();
        std::cout << "Root shape type: ";
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
        
        // Count using different methods
        std::cout << "\n=== Counting geometry elements ===" << std::endl;
        
        // Method 1: TopExp_Explorer (direct)
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
        
        std::cout << "Method 1 (TopExp_Explorer direct):" << std::endl;
        std::cout << "  Solids: " << solidCount << std::endl;
        std::cout << "  Shells: " << shellCount << std::endl;
        std::cout << "  Faces: " << faceCount << std::endl;
        std::cout << "  Edges: " << edgeCount << std::endl;
        std::cout << "  Vertices: " << vertexCount << std::endl;
        
        // Method 2: TopExp::MapShapes
        TopTools_IndexedMapOfShape solidMap, shellMap, faceMap, edgeMap, vertexMap;
        
        TopExp::MapShapes(shape, TopAbs_SOLID, solidMap);
        TopExp::MapShapes(shape, TopAbs_SHELL, shellMap);
        TopExp::MapShapes(shape, TopAbs_FACE, faceMap);
        TopExp::MapShapes(shape, TopAbs_EDGE, edgeMap);
        TopExp::MapShapes(shape, TopAbs_VERTEX, vertexMap);
        
        std::cout << "\nMethod 2 (TopExp::MapShapes):" << std::endl;
        std::cout << "  Solids: " << solidMap.Extent() << std::endl;
        std::cout << "  Shells: " << shellMap.Extent() << std::endl;
        std::cout << "  Faces: " << faceMap.Extent() << std::endl;
        std::cout << "  Edges: " << edgeMap.Extent() << std::endl;
        std::cout << "  Vertices: " << vertexMap.Extent() << std::endl;
        
        // If it's a compound, explore recursively
        if (shapeType == TopAbs_COMPOUND || shapeType == TopAbs_COMPSOLID) {
            std::cout << "\n=== Exploring COMPOUND structure ===" << std::endl;
            
            // Explore shells
            int shellIdx = 0;
            TopExp_Explorer shellExp2(shape, TopAbs_SHELL);
            for (; shellExp2.More(); shellExp2.Next()) {
                shellIdx++;
                const TopoDS_Shape& shell = shellExp2.Current();
                
                TopAbs_ShapeEnum shellType = shell.ShapeType();
                std::cout << "\nShell " << shellIdx << " type: ";
                switch (shellType) {
                    case TopAbs_SHELL: std::cout << "SHELL"; break;
                    case TopAbs_FACE: std::cout << "FACE (unexpected!)"; break;
                    default: std::cout << "UNKNOWN (" << shellType << ")"; break;
                }
                std::cout << std::endl;
                
                // Count faces in this shell
                int faceInShell = 0;
                TopExp_Explorer faceInShellExp(shell, TopAbs_FACE);
                for (; faceInShellExp.More(); faceInShellExp.Next()) faceInShell++;
                std::cout << "  Faces in shell: " << faceInShell << std::endl;
                
                // Count edges in this shell
                int edgeInShell = 0;
                TopExp_Explorer edgeInShellExp(shell, TopAbs_EDGE);
                for (; edgeInShellExp.More(); edgeInShellExp.Next()) edgeInShell++;
                std::cout << "  Edges in shell: " << edgeInShell << std::endl;
                
                // Explore faces in detail
                if (faceInShell > 0) {
                    int faceIdx = 0;
                    TopExp_Explorer faceDetailExp(shell, TopAbs_FACE);
                    for (; faceDetailExp.More(); faceDetailExp.Next()) {
                        faceIdx++;
                        const TopoDS_Shape& face = faceDetailExp.Current();
                        
                        // Count edges in this face
                        int edgeInFace = 0;
                        TopExp_Explorer edgeInFaceExp(face, TopAbs_EDGE);
                        for (; edgeInFaceExp.More(); edgeInFaceExp.Next()) edgeInFace++;
                        
                        std::cout << "  Face " << faceIdx << " has " << edgeInFace << " edges" << std::endl;
                    }
                }
            }
            
            std::cout << "\nTotal shells in compound: " << shellIdx << std::endl;
        }
        
        // Try recursive exploration
        std::cout << "\n=== Recursive exploration (TopExp_Explorer with TopAbs_SHAPE) ===" << std::endl;
        
        int totalShapes = 0;
        int shapesByType[TopAbs_SHAPE + 1] = {0};
        
        TopExp_Explorer allExp(shape, TopAbs_SHAPE);
        for (; allExp.More(); allExp.Next()) {
            totalShapes++;
            const TopoDS_Shape& currentShape = allExp.Current();
            TopAbs_ShapeEnum currentType = currentShape.ShapeType();
            
            if (currentType >= 0 && currentType <= TopAbs_SHAPE) {
                shapesByType[currentType]++;
            }
        }
        
        std::cout << "Total shapes found: " << totalShapes << std::endl;
        std::cout << "By type:" << std::endl;
        for (int i = 0; i <= TopAbs_SHAPE; i++) {
            if (shapesByType[i] > 0) {
                const char* typeName = "UNKNOWN";
                switch (i) {
                    case TopAbs_COMPOUND: typeName = "COMPOUND"; break;
                    case TopAbs_COMPSOLID: typeName = "COMPSOLID"; break;
                    case TopAbs_SOLID: typeName = "SOLID"; break;
                    case TopAbs_SHELL: typeName = "SHELL"; break;
                    case TopAbs_FACE: typeName = "FACE"; break;
                    case TopAbs_WIRE: typeName = "WIRE"; break;
                    case TopAbs_EDGE: typeName = "EDGE"; break;
                    case TopAbs_VERTEX: typeName = "VERTEX"; break;
                    case TopAbs_SHAPE: typeName = "SHAPE"; break;
                }
                std::cout << "  " << typeName << ": " << shapesByType[i] << std::endl;
            }
        }
        
        std::cout << "\n=== Test completed successfully ===" << std::endl;
        
    } catch (const Standard_Failure& e) {
        std::cout << "OpenCASCADE error: " << e.GetMessageString() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cout << "Standard error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cout << "Unknown error occurred" << std::endl;
        return 1;
    }
    
    return 0;
}