#include "ORBFeatureMatcher.h"
#include <chrono>
#include <iostream>
#include <cmath> // For sqrt
#include <map> // For grid distribution
#include <set> // For unique matches
#include <random> // For random_shuffle

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

        int h1 = sourceImage.rows;
        int w1 = sourceImage.cols;
        int h2 = templateImage.rows;
        int w2 = templateImage.cols;
        int max_dim = std::max(std::max(h1, w1), std::max(h2, w2));
    
        std::cout << "图像1尺寸: " << w1 << "x" << h1 << std::endl;
        std::cout << "图像2尺寸: " << w2 << "x" << h2 << std::endl;
        std::cout << "最大尺寸: " << max_dim << std::endl;

        int nfeatures = 2000;
        float scaleFactor = 1.2;
        int nlevels = 8;
        int edgeThreshold = 31;
        int fastThreshold = 20;
        double ratio_thresh = 0.75;
        double ransac_thresh = 1.5;
        // 自适应参数调整
        if (1)
        {
            if (max_dim > 1000)  // 高分辨率图像
            {
                nfeatures = 4000;  // 增加特征点数量
                scaleFactor = 1.25;  // 增加尺度因子
                nlevels = 12;  // 增加金字塔层级
                edgeThreshold = 20;  // 降低边缘阈值
                fastThreshold = 12;  // 降低FAST阈值
                ratio_thresh = 0.8;  // 放宽比率测试
                ransac_thresh = 2.0;  // 放宽RANSAC阈值
                std::cout << "使用高分辨率优化参数" << std::endl;
            }
            if (max_dim > 500)  // 中等分辨率图像
            {
                nfeatures = 2500;
                scaleFactor = 1.2;
                nlevels = 9;
                edgeThreshold = 28;
                fastThreshold = 18;
                ratio_thresh = 0.75;
                ransac_thresh = 1.2;
                std::cout << "使用中等分辨率优化参数" << std::endl;
            }
            else
            {                
                std::cout << "使用低分辨率优化参数" << std::endl;
            }
        }

        m_orb = cv::ORB::create(
            nfeatures, 
            scaleFactor, 
            nlevels, 
            edgeThreshold, 
            0, 
            2, 
            cv::ORB::HARRIS_SCORE, 
            31
        );

        // 先检测特征点
        m_orb->detect(sourceImage, kps);
        m_orb->detect(templateImage, kpt);
        
        // 对于高分辨率图像，进行特征点分布优化
        if (max_dim > 1000) {
            // 使用网格方法重新分布特征点
            auto distributeKeypoints = [](const std::vector<cv::KeyPoint>& keypoints, 
                                        const cv::Size& imgSize, int maxPoints = 4000) -> std::vector<cv::KeyPoint> {
                if (keypoints.size() <= maxPoints) {
                    return keypoints;
                }
                
                int h = imgSize.height;
                int w = imgSize.width;
                int gridSize = static_cast<int>(std::sqrt(maxPoints * w / h)); // 根据图像比例调整网格大小
                
                // 创建网格
                std::map<std::pair<int, int>, std::vector<cv::KeyPoint>> grid;
                for (const auto& kp : keypoints) {
                    int gridX = static_cast<int>(kp.pt.x / gridSize);
                    int gridY = static_cast<int>(kp.pt.y / gridSize);
                    auto gridKey = std::make_pair(gridX, gridY);
                    
                    if (grid.find(gridKey) == grid.end()) {
                        grid[gridKey] = std::vector<cv::KeyPoint>();
                    }
                    grid[gridKey].push_back(kp);
                }
                
                // 从每个网格中选择最佳特征点
                std::vector<cv::KeyPoint> distributedKps;
                for (const auto& gridCell : grid) {
                    // 按响应强度排序，选择最佳的点
                    std::vector<cv::KeyPoint> sortedCell = gridCell.second;
                    std::sort(sortedCell.begin(), sortedCell.end(), 
                             [](const cv::KeyPoint& a, const cv::KeyPoint& b) {
                                 return a.response > b.response;
                             });
                    
                    // 每个网格最多保留2个点
                    distributedKps.insert(distributedKps.end(), 
                                        sortedCell.begin(), 
                                        sortedCell.begin() + std::min(2, static_cast<int>(sortedCell.size())));
                }
                
                // 如果还是太多，随机选择
                if (distributedKps.size() > maxPoints) {
                    std::shuffle(distributedKps.begin(), distributedKps.end(), std::mt19937(std::random_device{}()));
                    distributedKps.resize(maxPoints);
                }
                
                return distributedKps;
            };
            
            kps = distributeKeypoints(kps, cv::Size(w1, h1));
            kpt = distributeKeypoints(kpt, cv::Size(w2, h2));
            std::cout << "特征点分布优化后: " << kps.size() << " -> " << kpt.size() << std::endl;
        }

        // 计算特征描述符
        m_orb->compute(sourceImage, kps, dess);
        m_orb->compute(templateImage, kpt, dest);
        
        auto t_orb_end = std::chrono::high_resolution_clock::now();
        double orb_ms = std::chrono::duration<double, std::milli>(t_orb_end - t_orb_start).count();
        std::cout << "ORB特征提取耗时: " << orb_ms << " ms" << std::endl;

        if (dest.empty() || dess.empty() || kpt.empty() || kps.empty()) {
            std::cerr << "特征提取失败！" << std::endl;
            return result;
        }

        std::cout << "图像1检测到 " << kps.size() << " 个特征点" << std::endl;
        std::cout << "图像2检测到 " << kpt.size() << " 个特征点" << std::endl;

        emit matchingProgress(10);
        
        // 2. 使用KNN匹配器进行比率测试（匹配Python代码逻辑）
        auto t_match_start = std::chrono::high_resolution_clock::now();
        cv::BFMatcher matcher(cv::NORM_HAMMING, false); // crossCheck=false for KNN
        
        // KNN匹配，k=2用于Lowe's ratio test
        std::vector<std::vector<cv::DMatch>> knn_matches;
        matcher.knnMatch(dess, dest, knn_matches, 2);

        auto t_match_end = std::chrono::high_resolution_clock::now();
        double match_ms = std::chrono::duration<double, std::milli>(t_match_end - t_match_start).count();
        std::cout << "特征匹配耗时: " << match_ms << " ms" << std::endl;

        if (knn_matches.size() < 10) {
            std::cerr << "匹配点太少！" << std::endl;
            return result;
        }
        
        emit matchingProgress(30);

        // 3. 应用Lowe's ratio test（匹配Python代码逻辑）
        auto t_sort_start = std::chrono::high_resolution_clock::now();
        
        std::vector<cv::DMatch> good_matches;
        for (const auto& matches : knn_matches) {
            if (matches.size() == 2) {
                const cv::DMatch& m = matches[0];
                const cv::DMatch& n = matches[1];
                if (m.distance < ratio_thresh * n.distance) { // 使用自适应比率
                    good_matches.push_back(m);
                }
            }
        }
        
        std::cout << "比率测试后的匹配点数量: " << good_matches.size() << std::endl;
        
        // 按距离排序
        std::sort(good_matches.begin(), good_matches.end(), [](const cv::DMatch& a, const cv::DMatch& b) {
            return a.distance < b.distance;
        });

        // 选择前N个最佳匹配点
        size_t N = std::min<size_t>(maxFeatures > 0 ? maxFeatures : 1000, good_matches.size());
        good_matches.resize(N);
        
        std::vector<cv::Point2f> s_pts, t_pts;
        for (const auto& m : good_matches) {
            s_pts.push_back(kps[m.queryIdx].pt);
            t_pts.push_back(kpt[m.trainIdx].pt);
        }
        
        result.goodMatches = good_matches;
        result.t_keypoints = kpt;
        result.s_keypoints = kps;
        
        std::cout << "有效匹配点数量: " << s_pts.size() << std::endl;
        
        emit matchingProgress(50);

        // 4. RANSAC筛选
        cv::Mat inlier_mask;
        if (s_pts.size() < 4 || t_pts.size() < 4) {
            std::cerr << "有效点太少！" << std::endl;
            return result;
        }
        
        auto t_ransac_start = std::chrono::high_resolution_clock::now();
        
        // 改进的RANSAC参数设置
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

        std::cout << "RANSAC筛选后的内点数量: " << inlier_matches.size() << std::endl;
        
        // 检查匹配的唯一性（匹配Python代码逻辑）
        std::set<int> unique_query_indices;
        std::set<int> unique_train_indices;
        std::vector<cv::DMatch> unique_matches;
        
        for (const auto& match : inlier_matches) {
            if (unique_query_indices.find(match.queryIdx) == unique_query_indices.end() && 
                unique_train_indices.find(match.trainIdx) == unique_train_indices.end()) {
                unique_query_indices.insert(match.queryIdx);
                unique_train_indices.insert(match.trainIdx);
                unique_matches.push_back(match);
            }
        }
        
        std::cout << "唯一匹配点数量: " << unique_matches.size() << std::endl;
        
        // 更新结果中的匹配点
        result.goodMatches = unique_matches;

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

        // 8. 计算匹配分数、旋转角度和缩放比例
        result.matchScore = static_cast<double>(unique_matches.size()) / good_matches.size();
        
        // 从单应性矩阵中提取旋转角度和缩放比例
        if (!H.empty()) {
            // 提取旋转角度
            double angle = atan2(H.at<double>(1, 0), H.at<double>(0, 0)) * 180.0 / CV_PI;
            result.rotationAngle = angle;
            
            // 提取缩放比例（使用矩阵的奇异值）
            cv::SVD svd(H);
            double scale_x = svd.w.at<double>(0);
            double scale_y = svd.w.at<double>(1);
            result.scale = (scale_x + scale_y) / 2.0;
            
            // 计算匹配位置（使用内点的中心）
            cv::Point2f center(0, 0);
            for (const auto& pt : inlier_pts1) {
                center += pt;
            }
            center.x /= static_cast<float>(inlier_pts1.size());
            center.y /= static_cast<float>(inlier_pts1.size());
            result.matchLocation = center;
        }
        
        // 9. 日志输出
        std::cout << "有效内点数: " << inlier_pts1.size() << " / " << good_matches.size() << std::endl;
        std::cout << "平均像素偏移: " << avg_pix_shift << " pixels" << std::endl;
        std::cout << "物理-像素比: " << scale_mm_per_pix << " mm/pixel" << std::endl;
        std::cout << "匹配分数: " << result.matchScore << std::endl;
        std::cout << "旋转角度: " << result.rotationAngle << "°" << std::endl;
        std::cout << "缩放比例: " << result.scale << std::endl;
        
        // 显示匹配统计信息（匹配Python代码）
        std::cout << "\n=== 匹配统计 ===" << std::endl;
        std::cout << "原始特征点: " << kps.size() << " -> " << kpt.size() << std::endl;
        std::cout << "比率测试后: " << good_matches.size() << std::endl;
        std::cout << "RANSAC筛选后: " << inlier_matches.size() << std::endl;
        std::cout << "唯一匹配: " << unique_matches.size() << std::endl;
        std::cout << "匹配成功率: " << (unique_matches.size() * 100.0 / good_matches.size()) << "%" << std::endl;

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