#pragma once

#include "DataStructures.h"
#include "SIMDOptimization.h"
#include <vector>
#include <memory>

// 模板匹配核心算法类
class TemplateMatcher
{
public:
    TemplateMatcher();
    ~TemplateMatcher();
    
    // 模板学习
    bool learnPattern(const cv::Mat& templateImage);
    
    // 执行匹配
    std::vector<s_SingleTargetMatch> match(const cv::Mat& sourceImage);
    
    // 参数设置
    void setMaxPositions(int maxPos) { m_iMaxPos = maxPos; }
    void setMaxOverlap(double maxOverlap) { m_dMaxOverlap = maxOverlap; }
    void setScore(double score) { m_dScore = score; }
    void setToleranceAngle(double angle) { m_dToleranceAngle = angle; }
    void setMinReduceArea(int area) { m_iMinReduceArea = area; }
    void setUseSIMD(bool useSIMD) { m_bUseSIMD = useSIMD; }
    void setSubPixelEstimation(bool enable) { m_bSubPixelEstimation = enable; }
    
    // 获取参数
    int getMaxPositions() const { return m_iMaxPos; }
    double getMaxOverlap() const { return m_dMaxOverlap; }
    double getScore() const { return m_dScore; }
    double getToleranceAngle() const { return m_dToleranceAngle; }
    int getMinReduceArea() const { return m_iMinReduceArea; }
    bool getUseSIMD() const { return m_bUseSIMD; }
    bool getSubPixelEstimation() const { return m_bSubPixelEstimation; }
    
    // 获取执行时间
    double getLastExecutionTime() const { return m_dLastExecutionTime; }
    
    // 检查模板是否已学习
    bool isPatternLearned() const { return m_TemplData.bIsPatternLearned; }
    
    // 清除模板数据
    void clearPattern();

private:
    // 核心算法函数
    int getTopLayer(const cv::Mat* matTempl, int iMinDstLength);
    void MatchTemplate(cv::Mat& matSrc, s_TemplData* pTemplData, cv::Mat& matResult, int iLayer, bool bUseSIMD);
    void getRotatedROI(cv::Mat& matSrc, cv::Size size, cv::Point2f ptLT, double dAngle, cv::Mat& matROI);
    void CCOEFF_Denominator(cv::Mat& matSrc, s_TemplData* pTemplData, cv::Mat& matResult, int iLayer);
    cv::Size getBestRotationSize(cv::Size sizeSrc, cv::Size sizeDst, double dRAngle);
    cv::Point2f ptRotatePt2f(cv::Point2f ptInput, cv::Point2f ptOrg, double dAngle);
    void filterWithScore(std::vector<s_MatchParameter>* vec, double dScore);
    void filterWithRotatedRect(std::vector<s_MatchParameter>* vec, int iMethod = cv::TM_CCOEFF_NORMED, double dMaxOverLap = 0);
    cv::Point getNextMaxLoc(cv::Mat& matResult, cv::Point ptMaxLoc, cv::Size sizeTemplate, double& dMaxValue, double dMaxOverlap);
    cv::Point getNextMaxLoc(cv::Mat& matResult, cv::Point ptMaxLoc, cv::Size sizeTemplate, double& dMaxValue, double dMaxOverlap, s_BlockMax& blockMax);
    bool subPixEstimation(std::vector<s_MatchParameter>* vec, double* dX, double* dY, double* dAngle, double dAngleStep, int iMaxScoreIndex);
    void sortPtWithCenter(std::vector<cv::Point2f>& vecSort);

    // 成员变量
    s_TemplData m_TemplData;
    cv::Mat m_sourceImage;  // 保存源图像用于角度计算
    int m_iMaxPos;
    double m_dMaxOverlap;
    double m_dScore;
    double m_dToleranceAngle;
    int m_iMinReduceArea;
    bool m_bUseSIMD;
    bool m_bSubPixelEstimation;
    double m_dLastExecutionTime;
    
    // 角度范围设置
    bool m_bToleranceRange;
    double m_dTolerance1, m_dTolerance2, m_dTolerance3, m_dTolerance4;
}; 