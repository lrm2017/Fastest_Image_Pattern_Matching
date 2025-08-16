#ifndef ORBFEATUREMATCHER_H
#define ORBFEATUREMATCHER_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <QImage>
#include <QDebug>

// ORB特征匹配结果结构
struct ORBMatchResult {
    cv::Point2f matchLocation;      // 匹配位置
    double matchScore;              // 匹配分数
    double rotationAngle;           // 旋转角度（度）
    double scale;                   // 缩放比例
    bool isMatched;                 // 是否匹配成功
    cv::Mat homographyMatrix;       // 单应性矩阵（用于精确变换）
    std::vector<cv::DMatch> goodMatches; // 好的匹配点
    std::vector<cv::KeyPoint> t_keypoints; // 模板图像关键点
    std::vector<cv::KeyPoint> s_keypoints; // 源图像关键点
};

class ORBFeatureMatcher : public QObject
{
    Q_OBJECT

public:
    explicit ORBFeatureMatcher(QObject *parent = nullptr);
    ~ORBFeatureMatcher();

    // 主要匹配方法
    ORBMatchResult performORBMatching(
                                    const cv::Mat& sourceImage, 
                                    const cv::Mat& templateImage,
                                     double matchThreshold = 0.7,
                                     int maxFeatures = 500,
                                     double physics_shift_mm = 8.0);

    // 设置ORB参数
    void setORBParameters(int maxFeatures = 500, 
                         float scaleFactor = 1.2f, 
                         int nLevels = 8,
                         int edgeThreshold = 31,
                         int firstLevel = 0,
                         int WTA_K = 2,
                         cv::ORB::ScoreType scoreType = cv::ORB::HARRIS_SCORE,
                         int patchSize = 31);

    // 设置匹配参数
    void setMatchingParameters(double matchThreshold = 0.7,
                              int maxGoodMatches = 150,
                              double ransacThreshold = 2.0,
                              int maxIterations = 2000,
                              double confidence = 0.99);

    // 获取匹配结果图像
    cv::Mat getMatchResultImage(
                                const cv::Mat& sourceImage,
                                const cv::Mat& templateImage,
                               const ORBMatchResult& result);

    // 计算模板在源图像中的四个角点位置
    std::vector<cv::Point2f> getTemplateCornersInSource(const cv::Mat& templateImage,
                                                        const ORBMatchResult& result);

    // 保存匹配结果
    bool saveMatchResult(const QString& filePath, const ORBMatchResult& result);

signals:
    // 匹配完成信号
    void matchingCompleted(const ORBMatchResult& result);
    
    // 匹配进度信号
    void matchingProgress(int progress);
    
    // 错误信号
    void matchingError(const QString& errorMessage);
    
    // 显示匹配结果图像信号
    void showMatchResultImage(const cv::Mat& resultImage, const QString& title);

private:
    // ORB特征检测器
    cv::Ptr<cv::ORB> m_orb;
    
    // 匹配参数
    double m_matchThreshold;
    int m_maxGoodMatches;
    double m_ransacThreshold;
    int m_maxIterations;
    double m_confidence;
    
    // 内部方法
    std::vector<cv::DMatch> filterMatches(const std::vector<cv::DMatch>& matches);
    cv::Mat findHomographyFromMatches(const std::vector<cv::KeyPoint>& kp1,
                                     const std::vector<cv::KeyPoint>& kp2,
                                     const std::vector<cv::DMatch>& matches);
    void drawMatchResult(cv::Mat& outputImage,
                        const cv::Mat& sourceImage,
                        const cv::Mat& templateImage,
                        const ORBMatchResult& result);
};

#endif // ORBFEATUREMATCHER_H 