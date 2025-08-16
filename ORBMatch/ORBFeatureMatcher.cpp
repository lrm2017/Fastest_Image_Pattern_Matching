#include "ORBFeatureMatcher.h"
#include <chrono>
#include <iostream>

ORBFeatureMatcher::ORBFeatureMatcher(QObject *parent)
    : QObject(parent)
    , m_matchThreshold(0.7)
    , m_maxGoodMatches(150)
    , m_ransacThreshold(2.0)
    , m_maxIterations(2000)
    , m_confidence(0.99)
{
    // 初始化ORB检测器
    m_orb = cv::ORB::create(500, 1.2f, 8, 31, 0, 2, cv::ORB::HARRIS_SCORE, 31);
}

ORBFeatureMatcher::~ORBFeatureMatcher()
{
}

ORBMatchResult ORBFeatureMatcher::performORBMatching(
                                    const cv::Mat& sourceImage, 
                                    const cv::Mat& templateImage,
                                    double matchThreshold,
                                    int maxFeatures,
                                    double physics_shift_mm)
{
    ORBMatchResult result;
    result.isMatched = false;
    
    try {
        // 检查输入图像
        if (sourceImage.empty() || templateImage.empty()) {
            emit matchingError("输入图像为空！");
            return result;
        }

        // 1. 提取ORB特征
        auto t_orb_start = std::chrono::high_resolution_clock::now();
        
        std::vector<cv::KeyPoint> kpt, kps;
        cv::Mat dest, dess;
        m_orb->detectAndCompute(sourceImage, cv::noArray(), kps, dess);
        m_orb->detectAndCompute(templateImage, cv::noArray(), kpt, dest);
        auto t_orb_end = std::chrono::high_resolution_clock::now();
        double orb_ms = std::chrono::duration<double, std::milli>(t_orb_end - t_orb_start).count();
        std::cout << "ORB特征提取耗时: " << orb_ms << " ms" << std::endl;

        if (dest.empty() || dess.empty() || kpt.empty() || kps.empty()) {
            std::cerr << "特征提取失败！" << std::endl;
            return result;
        }

        emit matchingProgress(10);
        
        // 2. 暴力匹配
        auto t_match_start = std::chrono::high_resolution_clock::now();
        cv::BFMatcher matcher(cv::NORM_HAMMING);
        std::vector<cv::DMatch> matches;
        matcher.match(dess, dest, matches);

        auto t_match_end = std::chrono::high_resolution_clock::now();
        double match_ms = std::chrono::duration<double, std::milli>(t_match_end - t_match_start).count();
        std::cout << "特征匹配耗时: " << match_ms << " ms" << std::endl;

        if (matches.size() < 10) {
            std::cerr << "匹配点太少！" << std::endl;
            return result;
        }
        
        emit matchingProgress(30);

        // 3. 特征匹配质量筛选
        auto t_sort_start = std::chrono::high_resolution_clock::now();
        
        std::sort(matches.begin(), matches.end(), [](const cv::DMatch& a, const cv::DMatch& b) {
            return a.distance < b.distance;
        });

        size_t N = std::min<size_t>(150, matches.size());
        std::vector<cv::Point2f> s_pts, t_pts;
        std::vector<cv::DMatch> good_matches(matches.begin(), matches.begin() + N);

        for (const auto& m : good_matches)
        {
            s_pts.push_back(kps[m.queryIdx].pt);
            t_pts.push_back(kpt[m.trainIdx].pt);
        }
        result.goodMatches = good_matches;
        result.t_keypoints = kpt;
        result.s_keypoints = kps;
        
        emit matchingProgress(50);

        // 4. RANSAC筛选
        cv::Mat inlier_mask;
        if (s_pts.size() < 4 || t_pts.size() < 4) {
            std::cerr << "有效点太少！" << std::endl;
            return result;
        }
        
        auto t_ransac_start = std::chrono::high_resolution_clock::now();
        
        // 改进的RANSAC参数设置
        double ransac_thresh = 2.0;  // 降低阈值，提高精度
        int max_iterations = 2000;    // 增加最大迭代次数
        double confidence = 0.99;     // 提高置信度
        
        cv::Mat H = cv::findHomography(s_pts, t_pts, cv::RANSAC, ransac_thresh, inlier_mask, max_iterations, confidence);
        
        auto t_ransac_end = std::chrono::high_resolution_clock::now();
        double ransac_ms = std::chrono::duration<double, std::milli>(t_ransac_end - t_ransac_start).count();
        std::cout << "RANSAC耗时: " << ransac_ms << " ms" << std::endl;
        
        if (H.empty()) {
            std::cerr << "RANSAC失败！" << std::endl;
            return result;
        }
        result.homographyMatrix = H;

        // 4.1. 验证单应矩阵的合理性
        std::cout << "\n单应矩阵 H:" << std::endl << H << std::endl;
        
        // 检查单应矩阵的条件数（衡量数值稳定性）
        cv::SVD svd_check(H);
        double condition_number = svd_check.w.at<double>(0) / svd_check.w.at<double>(2);
        std::cout << "单应矩阵条件数: " << condition_number << std::endl;
        
        if (condition_number > 1e6) {
            std::cout << "警告: 单应矩阵条件数过大，可能存在数值不稳定！" << std::endl;
        }
        
        // 检查变换是否保持方向性（行列式应该为正）
        double det = cv::determinant(H(cv::Rect(0, 0, 2, 2)));
        std::cout << "仿射部分行列式: " << det << std::endl;
        
        if (det < 0) {
            std::cout << "警告: 仿射变换改变了方向性，可能导致镜像效果！" << std::endl;
        }

        // 5. 从单应矩阵中提取内点匹配
        auto t_inlier_start = std::chrono::high_resolution_clock::now();
        std::vector<cv::DMatch> inlier_matches;
        std::vector<cv::Point2f> inlier_pts1, inlier_pts2;
        for (size_t i = 0; i < good_matches.size(); ++i) {
            if (inlier_mask.at<uchar>(i)) {
                inlier_matches.push_back(good_matches[i]);
                inlier_pts1.push_back(s_pts[i]);
                inlier_pts2.push_back(t_pts[i]);
            }
        }
        auto t_inlier_end = std::chrono::high_resolution_clock::now();
        double inlier_ms = std::chrono::duration<double, std::milli>(t_inlier_end - t_inlier_start).count();
        std::cout << "内点筛选耗时: " << inlier_ms << " ms" << std::endl;

        if (inlier_pts1.size() < 2) {
            std::cerr << "RANSAC内点太少！" << std::endl;
            return result;
        }

        result.isMatched = true;

        // 6. 计算平均像素偏移
        auto t_shift_start = std::chrono::high_resolution_clock::now();
        double total_shift = 0.0;
        for (size_t i = 0; i < inlier_pts1.size(); ++i) {
            cv::Point2f d = inlier_pts2[i] - inlier_pts1[i];
            total_shift += cv::norm(d);
        }
        double avg_pix_shift = total_shift / inlier_pts1.size();
        auto t_shift_end = std::chrono::high_resolution_clock::now();
        double shift_ms = std::chrono::duration<double, std::milli>(t_shift_end - t_shift_start).count();
        std::cout << "像素偏移计算耗时: " << shift_ms << " ms" << std::endl;

        if (avg_pix_shift < 1e-6) {
            std::cerr << "平均像素偏移过小，可能有误！" << std::endl;
            return result;
        }
        double physics_shift_mm = 8.0; // 可根据需要调整
        double scale_mm_per_pix = physics_shift_mm / avg_pix_shift;

        // 8. 日志输出
        std::cout << "有效内点数: " << inlier_pts1.size() << " / " << good_matches.size() << std::endl;
        std::cout << "平均像素偏移: " << avg_pix_shift << " pixels" << std::endl;
        std::cout << "物理-像素比: " << scale_mm_per_pix << " mm/pixel" << std::endl;

        // result.matchScore = 1.0;
        // result.rotationAngle = 0.0;
        // result.scale = scale_mm_per_pix;

        emit matchingProgress(100);
        emit matchingCompleted(result);
        
    } catch (const cv::Exception& e) {
        emit matchingError(QString("OpenCV错误: %1").arg(e.what()));
    } catch (const std::exception& e) {
        emit matchingError(QString("标准错误: %1").arg(e.what()));
    }
    
    return result;
}

void ORBFeatureMatcher::setORBParameters(int maxFeatures, float scaleFactor, int nLevels,
                                        int edgeThreshold, int firstLevel, int WTA_K,
                                        cv::ORB::ScoreType scoreType, int patchSize)
{
    m_orb = cv::ORB::create(maxFeatures, scaleFactor, nLevels, edgeThreshold, 
                           firstLevel, WTA_K, scoreType, patchSize);
}

void ORBFeatureMatcher::setMatchingParameters(double matchThreshold, int maxGoodMatches,
                                             double ransacThreshold, int maxIterations, double confidence)
{
    m_matchThreshold = matchThreshold;
    m_maxGoodMatches = maxGoodMatches;
    m_ransacThreshold = ransacThreshold;
    m_maxIterations = maxIterations;
    m_confidence = confidence;
}

std::vector<cv::DMatch> ORBFeatureMatcher::filterMatches(const std::vector<cv::DMatch>& matches)
{
    if (matches.empty()) return {};
    
    // 按距离排序
    std::vector<cv::DMatch> sortedMatches = matches;
    std::sort(sortedMatches.begin(), sortedMatches.end(), [](const cv::DMatch& a, const cv::DMatch& b) {
        return a.distance < b.distance;
    });
    
    size_t N = std::min<size_t>(150, sortedMatches.size());
    // std::vector<cv::Point2f> s_pts, t_pts;
    std::vector<cv::DMatch> goodMatches(sortedMatches.begin(), sortedMatches.begin() + N);

    return goodMatches;
}

cv::Mat ORBFeatureMatcher::findHomographyFromMatches(const std::vector<cv::KeyPoint>& kpt,
                                                     const std::vector<cv::KeyPoint>& kps,
                                                     const std::vector<cv::DMatch>& matches)
{
    if (matches.size() < 4) return cv::Mat();
    
    // 提取匹配点坐标
    std::vector<cv::Point2f> s_pts, t_pts;
    for (const auto& match : matches) {
        s_pts.push_back(kpt[match.queryIdx].pt);
        t_pts.push_back(kps[match.trainIdx].pt);
    }
    
    // 使用RANSAC计算单应性矩阵
    cv::Mat inlierMask;
    cv::Mat H = cv::findHomography(s_pts, t_pts, cv::RANSAC, 
                                   m_ransacThreshold, inlierMask, 
                                   m_maxIterations, m_confidence);
    
    return H;
}

cv::Mat ORBFeatureMatcher::getMatchResultImage(
                                                const cv::Mat& sourceImage,
                                                const cv::Mat& templateImage,
                                               const ORBMatchResult& result)
{
    if (!result.isMatched) {
        return sourceImage.clone();
    }
    
    // 创建结果图像 - 水平拼接源图像和模板图像
    cv::Mat outputImage;
    cv::drawMatches(
                   sourceImage, result.s_keypoints, 
                   templateImage, result.t_keypoints, 
                   result.goodMatches, outputImage,
                   cv::Scalar::all(-1), cv::Scalar::all(-1),
                   std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);
    
    // 在源图像上绘制匹配位置和匹配框
    if (result.isMatched) {
        // 获取模板在源图像中的角点位置
        std::vector<cv::Point2f> templateCorners = getTemplateCornersInSource(templateImage, result);
        
        if (templateCorners.size() == 4) {
            // 绘制匹配框
            for (int i = 0; i < 4; i++) {
                cv::line(outputImage, templateCorners[i], templateCorners[(i + 1) % 4], 
                        cv::Scalar(0, 255, 0), 3);
            }
            
            // 在匹配框中心绘制匹配点
            cv::circle(outputImage, result.matchLocation, 8, cv::Scalar(0, 0, 255), -1);
            cv::circle(outputImage, result.matchLocation, 12, cv::Scalar(255, 0, 0), 2);
            
            // 添加匹配信息文本
            std::string infoText = "Score: " + std::to_string(static_cast<int>(result.matchScore * 100)) + "%";
            cv::putText(outputImage, infoText, 
                       cv::Point(result.matchLocation.x + 20, result.matchLocation.y - 20),
                       cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);
            
            // 添加旋转角度信息
            std::string angleText = "Angle: " + std::to_string(static_cast<int>(result.rotationAngle)) + "°";
            cv::putText(outputImage, angleText, 
                       cv::Point(result.matchLocation.x + 20, result.matchLocation.y + 10),
                       cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 2);
            
            // 添加缩放比例信息
            std::string scaleText = "Scale: " + std::to_string(static_cast<int>(result.scale * 100)) + "%";
            cv::putText(outputImage, scaleText, 
                       cv::Point(result.matchLocation.x + 20, result.matchLocation.y + 30),
                       cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 255, 255), 2);
        }
        
        // 在图像顶部添加标题
        cv::putText(outputImage, "ORB Feature Matching Result", 
                   cv::Point(20, 40), cv::FONT_HERSHEY_SIMPLEX, 1.0, 
                   cv::Scalar(255, 255, 255), 2);
        
        // 在图像底部添加统计信息
        std::string statsText = "Matches: " + std::to_string(result.goodMatches.size()) + 
                               " | Inliers: " + std::to_string(result.t_keypoints.size());
        cv::putText(outputImage, statsText, 
                   cv::Point(20, outputImage.rows - 20), cv::FONT_HERSHEY_SIMPLEX, 0.7, 
                   cv::Scalar(255, 255, 255), 2);
    }
    
    return outputImage;
}

std::vector<cv::Point2f> ORBFeatureMatcher::getTemplateCornersInSource(const cv::Mat& templateImage,
                                                                       const ORBMatchResult& result)
{
    std::vector<cv::Point2f> corners;
    
    if (!result.isMatched) {
        return corners;
    }
    
    // 方法1：使用单应性矩阵进行精确变换
    // 标准公式：[x1, y1, 1]ᵀ = H × [x2, y2, 1]ᵀ
    if (!result.homographyMatrix.empty()) {
        // 计算模板图像的四个角点
        int width = templateImage.cols;
        int height = templateImage.rows;
        
        std::vector<cv::Point2f> templateCorners = {
            cv::Point2f(0, 0),                    // 左上角
            cv::Point2f(width, 0),                 // 右上角
            cv::Point2f(width, height),            // 右下角
            cv::Point2f(0, height)                 // 左下角
        };
        
        // 使用OpenCV的perspectiveTransform函数，它内部就是使用这个公式
        cv::perspectiveTransform(templateCorners, corners, result.homographyMatrix.inv());
        
        // 验证变换结果是否合理
        bool validTransform = true;
        for (const auto& point : corners) {
            if (point.x < -1000 || point.y < -1000 || 
                point.x > 10000 || point.y > 10000) { // 合理的图像尺寸上限
                validTransform = false;
                break;
            }
        }
        
        if (validTransform) {
            std::cout << "使用单应性矩阵变换角点成功" << std::endl;
            return corners;
        } else {
            std::cout << "警告: 单应性矩阵变换角点异常，回退到几何变换" << std::endl;
        }
    }
    
    // // 方法2：基于几何变换的坐标转换（备用方案）
    // // 计算模板图像的四个角点
    // int width = templateImage.cols;
    // int height = templateImage.rows;
    
    // std::vector<cv::Point2f> templateCorners = {
    //     cv::Point2f(0, 0),                    // 左上角
    //     cv::Point2f(width, 0),                 // 右上角
    //     cv::Point2f(width, height),            // 右下角
    //     cv::Point2f(0, height)                 // 左下角
    // };
    
    // // 计算模板图像的中心点
    // cv::Point2f templateImageCenter(width / 2.0f, height / 2.0f);
    
    // // 获取ORB匹配的中心位置
    // cv::Point2f matchCenter = result.matchLocation;
    
    // // 获取旋转角度和缩放比例
    // double angle = result.rotationAngle * CV_PI / 180.0;
    // double scale = result.scale;
    
    // // 预计算三角函数值
    // double cosA = cos(angle);
    // double sinA = sin(angle);
    
    // for (const auto& corner : templateCorners) {
    //     // 1. 将角点坐标转换为相对于模板图像中心的坐标
    //     cv::Point2f relativeCorner = corner - templateImageCenter;
        
    //     // 2. 应用旋转变换（先旋转）
    //     cv::Point2f rotatedCorner;
    //     rotatedCorner.x = relativeCorner.x * cosA - relativeCorner.y * sinA;
    //     rotatedCorner.y = relativeCorner.x * sinA + relativeCorner.y * cosA;
        
    //     // 3. 应用缩放变换（再缩放）
    //     rotatedCorner.x *= scale;
    //     rotatedCorner.y *= scale;
        
    //     // 4. 移动到匹配位置（最后平移）
    //     cv::Point2f sourceCorner = rotatedCorner + matchCenter;
    //     corners.push_back(sourceCorner);
    // }
    
    return corners;
}

bool ORBFeatureMatcher::saveMatchResult(const QString& filePath, const ORBMatchResult& result)
{
    if (!result.isMatched) return false;
    
    try {
        cv::FileStorage fs(filePath.toStdString(), cv::FileStorage::WRITE);
        if (!fs.isOpened()) return false;
        
        fs << "matchLocation_x" << result.matchLocation.x;
        fs << "matchLocation_y" << result.matchLocation.y;
        fs << "matchScore" << result.matchScore;
        fs << "rotationAngle" << result.rotationAngle;
        fs << "scale" << result.scale;
        fs << "isMatched" << result.isMatched;
        fs << "goodMatchesCount" << static_cast<int>(result.goodMatches.size());
        
        fs.release();
        return true;
    } catch (...) {
        return false;
    }
} 