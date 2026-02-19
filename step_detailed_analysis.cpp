#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>

// 解析STEP文件，详细列出所有实体类型
void analyzeStepFileDetailed(const std::string& filePath) {
    std::cout << "=== 详细STEP文件分析 ===" << std::endl;
    std::cout << "文件: " << filePath << std::endl;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << "错误: 无法打开文件" << std::endl;
        return;
    }
    
    std::string line;
    int lineNum = 0;
    std::map<std::string, int> entityCounts;
    std::vector<std::string> entityDefinitions;
    std::map<std::string, std::vector<std::string>> entityDetails;
    
    // 统计不同类型的实体
    while (std::getline(file, line)) {
        lineNum++;
        line.erase(0, line.find_first_not_of(" \t")); // 去除前导空白
        line.erase(line.find_last_not_of(" \t") + 1); // 去除尾部空白
        
        // 查找实体定义行，如 "#123 = ADVANCED_FACE(..."
        if (line.size() > 2 && line[0] == '#') {
            size_t equalsPos = line.find('=');
            if (equalsPos != std::string::npos) {
                std::string entityRef = line.substr(0, equalsPos);
                entityRef.erase(entityRef.find_last_not_of(" \t") + 1);
                
                std::string afterEquals = line.substr(equalsPos + 1);
                afterEquals.erase(0, afterEquals.find_first_not_of(" \t"));
                
                // 找到实体类型
                size_t typeEnd = afterEquals.find_first_of(" (");
                if (typeEnd != std::string::npos) {
                    std::string entityType = afterEquals.substr(0, typeEnd);
                    
                    // 清理类型字符串
                    entityType.erase(entityType.find_last_not_of(" \t") + 1);
                    
                    entityCounts[entityType]++;
                    entityDefinitions.push_back(entityRef + " = " + entityType);
                    
                    // 存储详细行
                    entityDetails[entityType].push_back(line);
                    
                    // 特殊处理几何相关实体
                    if (entityType == "ADVANCED_FACE" || entityType == "FACE_SURFACE" ||
                        entityType == "EDGE_CURVE" || entityType == "VERTEX_POINT" ||
                        entityType == "OPEN_SHELL" || entityType == "CLOSED_SHELL") {
                        std::cout << "行 " << lineNum << ": " << line << std::endl;
                    }
                }
            }
        }
        
        // 查找几何相关关键字
        if (line.find("ADVANCED_FACE") != std::string::npos ||
            line.find("FACE_SURFACE") != std::string::npos ||
            line.find("EDGE_CURVE") != std::string::npos ||
            line.find("VERTEX_POINT") != std::string::npos ||
            line.find("OPEN_SHELL") != std::string::npos ||
            line.find("CLOSED_SHELL") != std::string::npos ||
            line.find("PLANE") != std::string::npos ||
            line.find("CARTESIAN_POINT") != std::string::npos ||
            line.find("DIRECTION") != std::string::npos) {
            // 这些行已经在上面的处理中打印了，这里不再重复
        }
    }
    
    file.close();
    
    // 输出统计结果
    std::cout << "\n=== 实体类型统计 ===" << std::endl;
    for (const auto& pair : entityCounts) {
        std::cout << pair.first << ": " << pair.second << std::endl;
    }
    
    // 特别关注几何实体
    std::cout << "\n=== 几何实体详情 ===" << std::endl;
    std::vector<std::string> geometricEntities = {
        "ADVANCED_FACE", "FACE_SURFACE", "EDGE_CURVE", "VERTEX_POINT",
        "OPEN_SHELL", "CLOSED_SHELL", "PLANE", "CARTESIAN_POINT",
        "DIRECTION", "LINE", "CIRCLE", "AXIS2_PLACEMENT_3D"
    };
    
    int totalGeometricEntities = 0;
    for (const auto& entityType : geometricEntities) {
        if (entityCounts.find(entityType) != entityCounts.end()) {
            int count = entityCounts[entityType];
            totalGeometricEntities += count;
            std::cout << entityType << ": " << count << " 个" << std::endl;
            
            // 如果有实体，打印前几个作为示例
            if (count > 0 && count <= 5) {
                for (const auto& detail : entityDetails[entityType]) {
                    std::cout << "  " << detail << std::endl;
                }
            } else if (count > 5) {
                std::cout << "  (显示前5个)" << std::endl;
                for (int i = 0; i < 5 && i < entityDetails[entityType].size(); i++) {
                    std::cout << "  " << entityDetails[entityType][i] << std::endl;
                }
            }
        }
    }
    
    std::cout << "\n总几何实体数: " << totalGeometricEntities << std::endl;
    
    // 输出文件大小
    file.open(filePath, std::ios::binary | std::ios::ate);
    if (file.is_open()) {
        std::streamsize size = file.tellg();
        std::cout << "文件大小: " << size << " 字节" << std::endl;
        file.close();
    }
    
    // 简单验证几何数量
    std::cout << "\n=== 几何验证 ===" << std::endl;
    int faces = 0, edges = 0, vertices = 0, shells = 0;
    
    if (entityCounts.find("ADVANCED_FACE") != entityCounts.end())
        faces += entityCounts["ADVANCED_FACE"];
    if (entityCounts.find("FACE_SURFACE") != entityCounts.end())
        faces += entityCounts["FACE_SURFACE"];
    
    if (entityCounts.find("EDGE_CURVE") != entityCounts.end())
        edges = entityCounts["EDGE_CURVE"];
        
    if (entityCounts.find("VERTEX_POINT") != entityCounts.end())
        vertices = entityCounts["VERTEX_POINT"];
        
    if (entityCounts.find("OPEN_SHELL") != entityCounts.end())
        shells += entityCounts["OPEN_SHELL"];
    if (entityCounts.find("CLOSED_SHELL") != entityCounts.end())
        shells += entityCounts["CLOSED_SHELL"];
    
    std::cout << "面总数: " << faces << std::endl;
    std::cout << "边总数: " << edges << std::endl;
    std::cout << "顶点总数: " << vertices << std::endl;
    std::cout << "Shell总数: " << shells << std::endl;
    
    // 检查是否与期望的2个面、8条边匹配
    if (faces == 2 && edges == 8) {
        std::cout << "✓ 几何数量与期望一致 (2个面, 8条边)" << std::endl;
    } else {
        std::cout << "✗ 几何数量与期望不一致" << std::endl;
        std::cout << "  期望: 2个面, 8条边" << std::endl;
        std::cout << "  实际: " << faces << "个面, " << edges << "条边" << std::endl;
    }
    
    // 检查文件头和schema
    std::cout << "\n=== 文件头部信息 ===" << std::endl;
    file.open(filePath);
    for (int i = 0; i < 10 && std::getline(file, line); i++) {
        std::cout << line << std::endl;
    }
    file.close();
}

int main() {
    std::string filePath = "examples/two_sheetbodies.stp";
    
    try {
        analyzeStepFileDetailed(filePath);
        return 0;
    } catch (const std::exception& e) {
        std::cout << "错误: " << e.what() << std::endl;
        return 1;
    }
}