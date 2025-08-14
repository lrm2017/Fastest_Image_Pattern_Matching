#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <QString>
#include <QPointF>
#include <QRectF>

// 常量定义
#define VISION_TOLERANCE 0.0000001
#define D2R (CV_PI / 180.0)
#define R2D (180.0 / CV_PI)
#define MATCH_CANDIDATE_NUM 5

// 模板数据结构
struct s_TemplData
{
    std::vector<cv::Mat> vecPyramid;
    std::vector<cv::Scalar> vecTemplMean;
    std::vector<double> vecTemplNorm;
    std::vector<double> vecInvArea;
    std::vector<bool> vecResultEqual1;
    bool bIsPatternLearned;
    int iBorderColor;
    
    // 用户绘制的矩形区域
    cv::Rect userDefinedRect;
    bool hasUserRect;
    
    void clear()
    {
        std::vector<cv::Mat>().swap(vecPyramid);
        std::vector<double>().swap(vecTemplNorm);
        std::vector<double>().swap(vecInvArea);
        std::vector<cv::Scalar>().swap(vecTemplMean);
        std::vector<bool>().swap(vecResultEqual1);
        hasUserRect = false;
        userDefinedRect = cv::Rect();
    }
    
    void resize(int iSize)
    {
        vecTemplMean.resize(iSize);
        vecTemplNorm.resize(iSize, 0);
        vecInvArea.resize(iSize, 1);
        vecResultEqual1.resize(iSize, false);
    }
    
    s_TemplData()
    {
        bIsPatternLearned = false;
        hasUserRect = false;
        userDefinedRect = cv::Rect();
    }
};

// 匹配参数结构
struct s_MatchParameter
{
    cv::Point2d pt;
    double dMatchScore;
    double dMatchAngle;
    cv::Rect rectRoi;
    double dAngleStart;
    double dAngleEnd;
    cv::RotatedRect rectR;
    cv::Rect rectBounding;
    bool bDelete;

    double vecResult[3][3]; // for subpixel
    int iMaxScoreIndex;     // for subpixel
    bool bPosOnBorder;
    cv::Point2d ptSubPixel;
    double dNewAngle;

    s_MatchParameter(cv::Point2f ptMinMax, double dScore, double dAngle)
    {
        pt = ptMinMax;
        dMatchScore = dScore;
        dMatchAngle = dAngle;
        bDelete = false;
        dNewAngle = 0.0;
        bPosOnBorder = false;
    }
    
    s_MatchParameter()
    {
        dMatchScore = 0;
        dMatchAngle = 0;
        bDelete = false;
        bPosOnBorder = false;
        dNewAngle = 0.0;
    }
};

// 单目标匹配结果结构
struct s_SingleTargetMatch
{
    cv::Point2d ptLT, ptRT, ptRB, ptLB, ptCenter;
    double dMatchedAngle;
    double dMatchScore;
    
    // Qt兼容的转换函数
    QPointF getCenterQPoint() const {
        return QPointF(ptCenter.x, ptCenter.y);
    }
    
    QRectF getBoundingRect() const {
        double minX = std::min({ptLT.x, ptRT.x, ptRB.x, ptLB.x});
        double minY = std::min({ptLT.y, ptRT.y, ptRB.y, ptLB.y});
        double maxX = std::max({ptLT.x, ptRT.x, ptRB.x, ptLB.x});
        double maxY = std::max({ptLT.y, ptRT.y, ptRB.y, ptLB.y});
        return QRectF(minX, minY, maxX - minX, maxY - minY);
    }
};

// 分块最大值管理结构
struct s_BlockMax
{
    struct Block 
    {
        cv::Rect rect;
        double dMax;
        cv::Point ptMaxLoc;
        
        Block() {}
        Block(cv::Rect rect_, double dMax_, cv::Point ptMaxLoc_)
        {
            rect = rect_;
            dMax = dMax_;
            ptMaxLoc = ptMaxLoc_;
        }
    };
    
    std::vector<Block> vecBlock;
    cv::Mat matSrc;
    
    s_BlockMax() {}
    s_BlockMax(cv::Mat matSrc_, cv::Size sizeTemplate);
    void UpdateMax(cv::Rect rectIgnore);
    void GetMaxValueLoc(double& dMax, cv::Point& ptMaxLoc);
};

// 比较函数
bool compareScoreBig2Small(const s_MatchParameter& lhs, const s_MatchParameter& rhs);
bool compareMatchResultByScore(const s_SingleTargetMatch& lhs, const s_SingleTargetMatch& rhs);
bool compareMatchResultByPos(const s_SingleTargetMatch& lhs, const s_SingleTargetMatch& rhs);

// s_BlockMax类实现
inline s_BlockMax::s_BlockMax(cv::Mat matSrc_, cv::Size sizeTemplate)
{
    matSrc = matSrc_;
    
    // 计算分块大小
    int iBlockW = sizeTemplate.width;
    int iBlockH = sizeTemplate.height;
    
    int iCol = matSrc.cols / iBlockW;
    int iRow = matSrc.rows / iBlockH;
    
    // 创建分块
    for (int y = 0; y < iRow; y++) {
        for (int x = 0; x < iCol; x++) {
            cv::Rect rectBlock(x * iBlockW, y * iBlockH, iBlockW, iBlockH);
            cv::Mat matBlock = matSrc(rectBlock);
            
            double minVal, maxVal;
            cv::Point minLoc, maxLoc;
            cv::minMaxLoc(matBlock, &minVal, &maxVal, &minLoc, &maxLoc);
            
            vecBlock.push_back(Block(rectBlock, maxVal, cv::Point(rectBlock.x + maxLoc.x, rectBlock.y + maxLoc.y)));
        }
    }
    
    // 处理右侧剩余部分
    if (iCol * iBlockW < matSrc.cols) {
        cv::Rect rectRight(iCol * iBlockW, 0, matSrc.cols - iCol * iBlockW, matSrc.rows);
        cv::Mat matRight = matSrc(rectRight);
        
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(matRight, &minVal, &maxVal, &minLoc, &maxLoc);
        
        vecBlock.push_back(Block(rectRight, maxVal, cv::Point(rectRight.x + maxLoc.x, rectRight.y + maxLoc.y)));
    }
    
    // 处理底部剩余部分
    if (iRow * iBlockH < matSrc.rows) {
        cv::Rect rectBottom(0, iRow * iBlockH, iCol * iBlockW, matSrc.rows - iRow * iBlockH);
        cv::Mat matBottom = matSrc(rectBottom);
        
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(matBottom, &minVal, &maxVal, &minLoc, &maxLoc);
        
        vecBlock.push_back(Block(rectBottom, maxVal, cv::Point(rectBottom.x + maxLoc.x, rectBottom.y + maxLoc.y)));
    }
    
    // 处理右下角剩余部分
    if (iCol * iBlockW < matSrc.cols && iRow * iBlockH < matSrc.rows) {
        cv::Rect rectRight(iCol * iBlockW, 0, matSrc.cols - iCol * iBlockW, matSrc.rows);
        cv::Rect rectBottom(0, iRow * iBlockH, iCol * iBlockW, matSrc.rows - iRow * iBlockH);
        cv::Rect rectCorner(iCol * iBlockW, iRow * iBlockH, matSrc.cols - iCol * iBlockW, matSrc.rows - iRow * iBlockH);
        cv::Mat matCorner = matSrc(rectCorner);
        
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        cv::minMaxLoc(matCorner, &minVal, &maxVal, &minLoc, &maxLoc);
        
        vecBlock.push_back(Block(rectCorner, maxVal, cv::Point(rectCorner.x + maxLoc.x, rectCorner.y + maxLoc.y)));
    }
}

inline void s_BlockMax::UpdateMax(cv::Rect rectIgnore)
{
    // 更新最大值，忽略指定区域
    for (auto& block : vecBlock) {
        cv::Rect intersection = block.rect & rectIgnore;
        if (intersection.area() > 0) {
            // 重新计算这个块的最大值
            cv::Mat matBlock = matSrc(block.rect);
            double minVal, maxVal;
            cv::Point minLoc, maxLoc;
            cv::minMaxLoc(matBlock, &minVal, &maxVal, &minLoc, &maxLoc);
            
            block.dMax = maxVal;
            block.ptMaxLoc = cv::Point(block.rect.x + maxLoc.x, block.rect.y + maxLoc.y);
        }
    }
}

inline void s_BlockMax::GetMaxValueLoc(double& dMax, cv::Point& ptMaxLoc)
{
    if (vecBlock.empty()) {
        dMax = -1;
        ptMaxLoc = cv::Point(-1, -1);
        return;
    }
    
    // 找到最大值
    auto maxBlock = std::max_element(vecBlock.begin(), vecBlock.end(), 
        [](const Block& a, const Block& b) { return a.dMax < b.dMax; });
    
    dMax = maxBlock->dMax;
    ptMaxLoc = maxBlock->ptMaxLoc;
} 