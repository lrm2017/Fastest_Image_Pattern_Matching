#!/usr/bin/env python3
"""
测试图像匹配功能的Python脚本
"""

import cv2
import numpy as np
import os
import sys

def test_template_matching():
    """测试模板匹配功能"""
    
    # 检查测试图像是否存在
    test_dir = "Test Images"
    if not os.path.exists(test_dir):
        print(f"错误: 测试图像目录 {test_dir} 不存在")
        return False
    
    # 查找测试图像
    source_files = [f for f in os.listdir(test_dir) if f.endswith(('.bmp', '.jpg', '.png')) and 'Src' in f]
    template_files = [f for f in os.listdir(test_dir) if f.endswith(('.bmp', '.jpg', '.png')) and 'Dst' in f]
    
    if not source_files or not template_files:
        print("错误: 未找到测试图像文件")
        return False
    
    # 选择第一个源图像和模板图像
    source_path = os.path.join(test_dir, source_files[0])
    template_path = os.path.join(test_dir, template_files[0])
    
    print(f"使用源图像: {source_path}")
    print(f"使用模板图像: {template_path}")
    
    # 读取图像
    source = cv2.imread(source_path, cv2.IMREAD_GRAYSCALE)
    template = cv2.imread(template_path, cv2.IMREAD_GRAYSCALE)
    
    if source is None or template is None:
        print("错误: 无法读取图像文件")
        return False
    
    print(f"源图像尺寸: {source.shape}")
    print(f"模板图像尺寸: {template.shape}")
    
    # 执行模板匹配
    print("\n执行模板匹配...")
    
    # 使用OpenCV的模板匹配
    result = cv2.matchTemplate(source, template, cv2.TM_CCOEFF_NORMED)
    
    # 找到最佳匹配
    min_val, max_val, min_loc, max_loc = cv2.minMaxLoc(result)
    
    print(f"最佳匹配分数: {max_val:.4f}")
    print(f"最佳匹配位置: {max_loc}")
    
    # 设置阈值
    threshold = 0.5
    print(f"使用阈值: {threshold}")
    
    # 找到所有超过阈值的匹配
    locations = np.where(result >= threshold)
    matches = list(zip(*locations[::-1]))  # 转换为(x, y)格式
    
    print(f"找到 {len(matches)} 个超过阈值的匹配")
    
    # 显示前5个最佳匹配
    for i, (x, y) in enumerate(matches[:5]):
        score = result[y, x]
        print(f"匹配 {i+1}: 位置=({x}, {y}), 分数={score:.4f}")
    
    # 创建结果图像
    result_img = cv2.cvtColor(source, cv2.COLOR_GRAY2BGR)
    
    # 绘制匹配结果
    for i, (x, y) in enumerate(matches[:5]):
        # 绘制矩形框
        cv2.rectangle(result_img, (x, y), (x + template.shape[1], y + template.shape[0]), (0, 255, 0), 2)
        
        # 绘制序号
        cv2.putText(result_img, str(i+1), (x+5, y+20), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255, 255, 0), 2)
    
    # 保存结果图像
    output_path = "matching_result.jpg"
    cv2.imwrite(output_path, result_img)
    print(f"\n结果图像已保存到: {output_path}")
    
    return True

def main():
    """主函数"""
    print("=== 图像模板匹配测试 ===")
    
    if test_template_matching():
        print("\n测试完成!")
    else:
        print("\n测试失败!")
        sys.exit(1)

if __name__ == "__main__":
    main() 