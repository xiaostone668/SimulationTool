#!/usr/bin/env python3
"""
快速分析STEP文件中的几何元素数量
验证two_sheetbodies.stp是否有2个面和8条边
"""
import re
import sys

def count_step_elements(file_path):
    """统计STEP文件中的面和边数量"""
    with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
        content = f.read()
    
    # 统计ADVANCED_FACE数量
    advanced_face_count = len(re.findall(r'#\d+\s*=\s*ADVANCED_FACE', content))
    face_surface_count = len(re.findall(r'#\d+\s*=\s*FACE_SURFACE', content))
    total_faces = advanced_face_count + face_surface_count
    
    # 统计EDGE_CURVE数量
    edge_curve_count = len(re.findall(r'#\d+\s*=\s*EDGE_CURVE', content))
    
    # 统计VERTEX_POINT数量
    vertex_count = len(re.findall(r'#\d+\s*=\s*VERTEX_POINT', content))
    
    # 统计SHELL数量
    open_shell_count = len(re.findall(r'#\d+\s*=\s*OPEN_SHELL', content))
    closed_shell_count = len(re.findall(r'#\d+\s*=\s*CLOSED_SHELL', content))
    total_shells = open_shell_count + closed_shell_count
    
    print(f"文件: {file_path}")
    print(f"ADVANCED_FACE 数量: {advanced_face_count}")
    print(f"FACE_SURFACE 数量: {face_surface_count}")
    print(f"总面数: {total_faces}")
    print(f"EDGE_CURVE 数量: {edge_curve_count}")
    print(f"VERTEX_POINT 数量: {vertex_count}")
    print(f"OPEN_SHELL 数量: {open_shell_count}")
    print(f"CLOSED_SHELL 数量: {closed_shell_count}")
    print(f"总SHELL数: {total_shells}")
    print()
    
    # 验证要求
    print("=== 验证结果 ===")
    if total_faces == 2:
        print(f"✓ 面数量正确: {total_faces} 个面")
    else:
        print(f"✗ 面数量错误: 期望2个，实际{total_faces}个")
    
    if edge_curve_count == 8:
        print(f"✓ 边数量正确: {edge_curve_count} 条边")
    else:
        print(f"✗ 边数量错误: 期望8条，实际{edge_curve_count}条")
    
    return total_faces, edge_curve_count

if __name__ == "__main__":
    file_path = "examples/two_sheetbodies.stp"
    
    try:
        faces, edges = count_step_elements(file_path)
        sys.exit(0 if faces == 2 and edges == 8 else 1)
    except FileNotFoundError:
        print(f"错误: 找不到文件 {file_path}")
        sys.exit(1)
    except Exception as e:
        print(f"错误: {e}")
        sys.exit(1)