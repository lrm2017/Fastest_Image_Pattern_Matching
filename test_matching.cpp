#include "include/TemplateMatcher.h"
#include <opencv2/opencv.hpp>
#include <iostream>

int main()
{
    // 创建模板匹配器
    TemplateMatcher matcher;
    
    // 设置参数
    matcher.setScore(0.7);
    matcher.setMaxPositions(10);
    matcher.setToleranceAngle(5.0);
    
    // 加载模板图像
    cv::Mat templateImage = cv::imread("Test Images/Src1.bmp", cv::IMREAD_GRAYSCALE);
    if (templateImage.empty()) {
        std::cout << "无法加载模板图像" << std::endl;
        return -1;
    }
    
    // 学习模板
    if (!matcher.learnPattern(templateImage)) {
        std::cout << "模板学习失败" << std::endl;
        return -1;
    }
    
    std::cout << "模板学习成功，模板尺寸: " << templateImage.cols << "x" << templateImage.rows << std::endl;
    
    // 加载源图像
    cv::Mat sourceImage = cv::imread("Test Images/Dst1.bmp", cv::IMREAD_GRAYSCALE);
    if (sourceImage.empty()) {
        std::cout << "无法加载源图像" << std::endl;
        return -1;
    }
    
    std::cout << "源图像尺寸: " << sourceImage.cols << "x" << sourceImage.rows << std::endl;
    
    // 执行匹配
    std::vector<s_SingleTargetMatch> results = matcher.match(sourceImage);
    
    // 显示结果
    std::cout << "找到 " << results.size() << " 个匹配结果:" << std::endl;
    
    for (size_t i = 0; i < results.size(); i++) {
        const auto& result = results[i];
        std::cout << "结果 " << i + 1 << ":" << std::endl;
        std::cout << "  中心点: (" << result.ptCenter.x << ", " << result.ptCenter.y << ")" << std::endl;
        std::cout << "  匹配分数: " << result.dMatchScore << std::endl;
        std::cout << "  匹配角度: " << result.dMatchedAngle << " 度" << std::endl;
        std::cout << "  左上角: (" << result.ptLT.x << ", " << result.ptLT.y << ")" << std::endl;
        std::cout << "  右上角: (" << result.ptRT.x << ", " << result.ptRT.y << ")" << std::endl;
        std::cout << "  左下角: (" << result.ptLB.x << ", " << result.ptLB.y << ")" << std::endl;
        std::cout << "  右下角: (" << result.ptRB.x << ", " << result.ptRB.y << ")" << std::endl;
        std::cout << std::endl;
    }
    
    std::cout << "执行时间: " << matcher.getLastExecutionTime() << " 秒" << std::endl;
    
    return 0;
} 