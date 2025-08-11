#include "TemplateMatcher.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>
#include "SIMDOptimization.h"
#include <immintrin.h>
#include <cfloat>
#include <QDebug>

// 添加缺失的比较函数实现
bool compareScoreBig2Small(const s_MatchParameter& lhs, const s_MatchParameter& rhs) 
{ 
    return lhs.dMatchScore > rhs.dMatchScore; 
}
bool comparePtWithAngle (const std::pair<cv::Point2f, double> lhs, const std::pair<cv::Point2f, double> rhs) { return lhs.second < rhs.second; }

// 添加_mm_hsum_epi32的实现
inline int _mm_hsum_epi32(__m128i V)
{
    __m128i T = _mm_add_epi32(V, _mm_srli_si128(V, 8));
    T = _mm_add_epi32(T, _mm_srli_si128(T, 4));
    return _mm_cvtsi128_si32(T);
}

TemplateMatcher::TemplateMatcher()
    : m_iMaxPos(70)
    , m_dMaxOverlap(0.0)
    , m_dScore(0.7)  // 调整阈值，确保能匹配到0.799767的结果
    , m_dToleranceAngle(0.0)
    , m_iMinReduceArea(256)
    , m_bUseSIMD(true)
    , m_bSubPixelEstimation(false)
    , m_dLastExecutionTime(0.0)
{
    m_bToleranceRange = false;
}

TemplateMatcher::~TemplateMatcher()
{
}

bool TemplateMatcher::learnPattern(const cv::Mat& templateImage)
{
    if (templateImage.empty()) {
        return false;
    }
    
    m_TemplData.clear();
    
    // 构建金字塔
    int iTopLayer = getTopLayer(&templateImage, (int)sqrt((double)m_iMinReduceArea));
    cv::buildPyramid(templateImage, m_TemplData.vecPyramid, iTopLayer);
    
    // 设置边框颜色
    cv::Scalar meanColor = cv::mean(templateImage);
    m_TemplData.iBorderColor = meanColor.val[0] < 128 ? 255 : 0;
    
    // 调整大小
    int iSize = m_TemplData.vecPyramid.size();
    m_TemplData.resize(iSize);
    
    // 计算统计信息 - 修复为与MFC版本一致
    for (int i = 0; i < iSize; i++) {
        double invArea = 1.0 / ((double)m_TemplData.vecPyramid[i].rows * m_TemplData.vecPyramid[i].cols);
        cv::Scalar templMean, templSdv;
        double templNorm = 0, templSum2 = 0;
        
        cv::meanStdDev(m_TemplData.vecPyramid[i], templMean, templSdv);
        
        // 修复：使用与MFC版本相同的计算方式
        templNorm = templSdv[0] * templSdv[0] + templSdv[1] * templSdv[1] + 
                   templSdv[2] * templSdv[2] + templSdv[3] * templSdv[3];
        
        if (templNorm < DBL_EPSILON) {
            m_TemplData.vecResultEqual1[i] = true;
        }
        
        templSum2 = templNorm + templMean[0] * templMean[0] + templMean[1] * templMean[1] + 
                   templMean[2] * templMean[2] + templMean[3] * templMean[3];
        
        templSum2 /= invArea;
        templNorm = std::sqrt(templNorm);
        templNorm /= std::sqrt(invArea); // 注意精度
        
        m_TemplData.vecInvArea[i] = invArea;
        m_TemplData.vecTemplMean[i] = templMean;
        m_TemplData.vecTemplNorm[i] = templNorm;
    }
    
    m_TemplData.bIsPatternLearned = true;
    return true;
}

std::vector<s_SingleTargetMatch> TemplateMatcher::match(const cv::Mat& sourceImage)
{
    if (sourceImage.empty() || !m_TemplData.bIsPatternLearned) {
        return std::vector<s_SingleTargetMatch>();
    }
    
    // 保存源图像引用用于角度计算
    m_sourceImage = sourceImage.clone();
    
    // 检查图像尺寸
    if ((m_TemplData.vecPyramid[0].cols < sourceImage.cols && m_TemplData.vecPyramid[0].rows > sourceImage.rows) || 
        (m_TemplData.vecPyramid[0].cols > sourceImage.cols && m_TemplData.vecPyramid[0].rows < sourceImage.rows)) {
        return std::vector<s_SingleTargetMatch>();
    }
    
    if (m_TemplData.vecPyramid[0].size().area() > sourceImage.size().area()) {
        return std::vector<s_SingleTargetMatch>();
    }
    
    // 记录开始时间
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 决定金字塔层数
    int iTopLayer = getTopLayer(&m_TemplData.vecPyramid[0], (int)sqrt((double)m_iMinReduceArea));
    
    // 构建源图像金字塔
    std::vector<cv::Mat> vecMatSrcPyr;
    cv::buildPyramid(sourceImage, vecMatSrcPyr, iTopLayer);

    
    s_TemplData* pTemplData = &m_TemplData;
    
    // 第一阶段：以最顶层找出大致角度与ROI
    double dAngleStep = atan(2.0 / std::max(pTemplData->vecPyramid[iTopLayer].cols, pTemplData->vecPyramid[iTopLayer].rows)) * R2D;
    
    std::vector<double> vecAngles;
    
    // 角度范围设置
    if (m_dToleranceAngle < VISION_TOLERANCE) {
        vecAngles.push_back(0.0);
    } else {
        for (double dAngle = 0; dAngle < m_dToleranceAngle + dAngleStep; dAngle += dAngleStep) {
            vecAngles.push_back(dAngle);
        }
        for (double dAngle = -dAngleStep; dAngle > -m_dToleranceAngle - dAngleStep; dAngle -= dAngleStep) {
            vecAngles.push_back(dAngle);
        }
    }
    
    int iTopSrcW = vecMatSrcPyr[iTopLayer].cols, iTopSrcH = vecMatSrcPyr[iTopLayer].rows;
    cv::Point2f ptCenter((iTopSrcW - 1) / 2.0f, (iTopSrcH - 1) / 2.0f);
    
    int iSize = (int)vecAngles.size();
    std::vector<s_MatchParameter> vecMatchParameter;
    
    // 计算每层的最低分数
    std::vector<double> vecLayerScore(iTopLayer + 1, m_dScore);
    for (int iLayer = 1; iLayer <= iTopLayer; iLayer++) {
        vecLayerScore[iLayer] = vecLayerScore[iLayer - 1] * 0.9;
    }
    
    cv::Size sizePat = pTemplData->vecPyramid[iTopLayer].size();
    bool bCalMaxByBlock = (vecMatSrcPyr[iTopLayer].size().area() / sizePat.area() > 500) && m_iMaxPos > 10;
    
    // 对每个角度进行匹配
    for (int i = 0; i < iSize; i++) {
        cv::Mat matRotatedSrc, matR = cv::getRotationMatrix2D(ptCenter, vecAngles[i], 1);
        cv::Mat matResult;
        cv::Point ptMaxLoc;
        double dValue, dMaxVal;
        
        cv::Size sizeBest = getBestRotationSize(vecMatSrcPyr[iTopLayer].size(), pTemplData->vecPyramid[iTopLayer].size(), vecAngles[i]);
        
        float fTranslationX = (sizeBest.width - 1) / 2.0f - ptCenter.x;
        float fTranslationY = (sizeBest.height - 1) / 2.0f - ptCenter.y;
        matR.at<double>(0, 2) += fTranslationX;
        matR.at<double>(1, 2) += fTranslationY;
        
        cv::warpAffine(vecMatSrcPyr[iTopLayer], matRotatedSrc, matR, sizeBest, cv::INTER_LINEAR, cv::BORDER_CONSTANT, cv::Scalar(pTemplData->iBorderColor));
        
        MatchTemplate(matRotatedSrc, pTemplData, matResult, iTopLayer, false);
        
        if (bCalMaxByBlock) {
            // 使用分块最大值计算
            s_BlockMax blockMax(matResult, pTemplData->vecPyramid[iTopLayer].size());
            blockMax.GetMaxValueLoc(dMaxVal, ptMaxLoc);
            if (dMaxVal < vecLayerScore[iTopLayer]) {
                continue;
            }
            vecMatchParameter.push_back(s_MatchParameter(cv::Point2f(ptMaxLoc.x - fTranslationX, ptMaxLoc.y - fTranslationY), dMaxVal, vecAngles[i]));
            
            for (int j = 0; j < m_iMaxPos + MATCH_CANDIDATE_NUM - 1; j++) {
                ptMaxLoc = getNextMaxLoc(matResult, ptMaxLoc, pTemplData->vecPyramid[iTopLayer].size(), dValue, m_dMaxOverlap, blockMax);
                if (dValue < vecLayerScore[iTopLayer]) {
                    break;
                }
                vecMatchParameter.push_back(s_MatchParameter(cv::Point2f(ptMaxLoc.x - fTranslationX, ptMaxLoc.y - fTranslationY), dValue, vecAngles[i]));
            }
        } else {
            // 使用标准方法
            cv::minMaxLoc(matResult, nullptr, &dMaxVal, nullptr, &ptMaxLoc);
            if (dMaxVal < vecLayerScore[iTopLayer]) {
                continue;
            }
            vecMatchParameter.push_back(s_MatchParameter(cv::Point2f(ptMaxLoc.x - fTranslationX, ptMaxLoc.y - fTranslationY), dMaxVal, vecAngles[i]));
            
            for (int j = 0; j < m_iMaxPos + MATCH_CANDIDATE_NUM - 1; j++) {
                ptMaxLoc = getNextMaxLoc(matResult, ptMaxLoc, pTemplData->vecPyramid[iTopLayer].size(), dValue, m_dMaxOverlap);
                if (dValue < vecLayerScore[iTopLayer]) {
                    break;
                }
                vecMatchParameter.push_back(s_MatchParameter(cv::Point2f(ptMaxLoc.x - fTranslationX, ptMaxLoc.y - fTranslationY), dValue, vecAngles[i]));
            }
        }
    }
    
    // 按分数排序
    std::sort(vecMatchParameter.begin(), vecMatchParameter.end(), compareScoreBig2Small);
    
    int iMatchSize = (int)vecMatchParameter.size ();
	int iDstW = pTemplData->vecPyramid[iTopLayer].cols, iDstH = pTemplData->vecPyramid[iTopLayer].rows;

    //顯示第一層結果 (调试模式)
	//if (1)//m_bDebugMode
	//{
	//	int iDebugScale = 2;
	//
	//	cv::Mat matShow, matResize;
	//	cv::resize (vecMatSrcPyr[iTopLayer], matResize, vecMatSrcPyr[iTopLayer].size () * iDebugScale);
	//	cv::cvtColor (matResize, matShow, cv::COLOR_GRAY2BGR);
	//	std::string str = cv::format ("Toplayer, Candidate:%d", iMatchSize);
	//	std::vector<cv::Point2f> vec;
	//	for (int i = 0; i < iMatchSize; i++)
	//	{
	//		cv::Point2f ptLT, ptRT, ptRB, ptLB;
	//		double dRAngle = -vecMatchParameter[i].dMatchAngle * D2R;
	//		ptLT = ptRotatePt2f (vecMatchParameter[i].pt, ptCenter, dRAngle);
	//		ptRT = cv::Point2f (ptLT.x + iDstW * (float)cos (dRAngle), ptLT.y - iDstW * (float)sin (dRAngle));
	//		ptLB = cv::Point2f (ptLT.x + iDstH * (float)sin (dRAngle), ptLT.y + iDstH * (float)cos (dRAngle));
	//		ptRB = cv::Point2f (ptRT.x + iDstH * (float)sin (dRAngle), ptRT.y + iDstH * (float)cos (dRAngle));
	//		cv::line (matShow, ptLT * iDebugScale, ptLB * iDebugScale, cv::Scalar (0, 255, 0));
	//		cv::line (matShow, ptLB * iDebugScale, ptRB * iDebugScale, cv::Scalar (0, 255, 0));
	//		cv::line (matShow, ptRB * iDebugScale, ptRT * iDebugScale, cv::Scalar (0, 255, 0));
	//		cv::line (matShow, ptRT * iDebugScale, ptLT * iDebugScale, cv::Scalar (0, 255, 0));
	//		cv::circle (matShow, ptLT * iDebugScale, 1, cv::Scalar (0, 0, 255));
	//		vec.push_back (ptLT* iDebugScale);
	//		vec.push_back (ptRT* iDebugScale);
	//		vec.push_back (ptLB * iDebugScale);
	//		vec.push_back (ptRB * iDebugScale);
	//
	//		std::string strText = cv::format ("%d", i);
	//		cv::putText (matShow, strText, ptLT *iDebugScale, cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar (0, 255, 0));
	//	}
	//	cv::namedWindow (str.c_str (), cv::WINDOW_NORMAL);
	//	cv::Rect rectShow = cv::boundingRect (vec);
	//	cv::imshow (str, matShow);// (rectShow));
	//	//moveWindow (str, 0, 0);
	//}
    //展示第一层结果

	//第一阶段结束
	bool bSubPixelEstimation = 0;//m_bSubPixel.GetCheck ();
	int iStopLayer = 0;//m_bStopLayer1 ? 1 : 0; //设置为1时：粗匹配，牺牲精度提升速度。
	//int iSearchSize = min (m_iMaxPos + MATCH_CANDIDATE_NUM, (int)vecMatchParameter.size ());//可能不需要搜尋到全部 太浪費時間
	std::vector<s_MatchParameter> vecAllResult;
	for (int i = 0; i < (int)vecMatchParameter.size (); i++)
	//for (int i = 0; i < iSearchSize; i++)
	{
		double dRAngle = -vecMatchParameter[i].dMatchAngle * D2R;
		cv::Point2f ptLT = ptRotatePt2f (vecMatchParameter[i].pt, ptCenter, dRAngle);

		double dAngleStep = atan (2.0 / std::max (iDstW, iDstH)) * R2D;//min改為max
		vecMatchParameter[i].dAngleStart = vecMatchParameter[i].dMatchAngle - dAngleStep;
		vecMatchParameter[i].dAngleEnd = vecMatchParameter[i].dMatchAngle + dAngleStep;

		if (iTopLayer <= iStopLayer)
		{
			vecMatchParameter[i].pt = cv::Point2d (ptLT * ((iTopLayer == 0) ? 1 : 2));
			vecAllResult.push_back (vecMatchParameter[i]);
		}
		else
		{
			for (int iLayer = iTopLayer - 1; iLayer >= iStopLayer; iLayer--)
			{
				//搜尋角度
				dAngleStep = atan (2.0 / std::max (pTemplData->vecPyramid[iLayer].cols, pTemplData->vecPyramid[iLayer].rows)) * R2D;//min改為max
				std::vector<double> vecAngles;
				//double dAngleS = vecMatchParameter[i].dAngleStart, dAngleE = vecMatchParameter[i].dAngleEnd;
				double dMatchedAngle = vecMatchParameter[i].dMatchAngle;
				if (m_bToleranceRange)
				{
					for (int i = -1; i <= 1; i++)
						vecAngles.push_back (dMatchedAngle + dAngleStep * i);
				}
				else
				{
					if (m_dToleranceAngle < VISION_TOLERANCE)
						vecAngles.push_back (0.0);
					else
						for (int i = -1; i <= 1; i++)
							vecAngles.push_back (dMatchedAngle + dAngleStep * i);
				}
				cv::Point2f ptSrcCenter ((vecMatSrcPyr[iLayer].cols - 1) / 2.0f, (vecMatSrcPyr[iLayer].rows - 1) / 2.0f);
				iSize = (int)vecAngles.size ();
				std::vector<s_MatchParameter> vecNewMatchParameter (iSize);
				int iMaxScoreIndex = 0;
				double dBigValue = -1;
				for (int j = 0; j < iSize; j++)
				{
					cv::Mat matResult, matRotatedSrc;
					double dMaxValue = 0;
					cv::Point ptMaxLoc;
					getRotatedROI (vecMatSrcPyr[iLayer], pTemplData->vecPyramid[iLayer].size (), ptLT * 2, vecAngles[j], matRotatedSrc);

					MatchTemplate (matRotatedSrc, pTemplData, matResult, iLayer, true);
					cv::minMaxLoc (matResult, 0, &dMaxValue, 0, &ptMaxLoc);
					vecNewMatchParameter[j] = s_MatchParameter (ptMaxLoc, dMaxValue, vecAngles[j]);
					
					if (vecNewMatchParameter[j].dMatchScore > dBigValue)
					{
						iMaxScoreIndex = j;
						dBigValue = vecNewMatchParameter[j].dMatchScore;
					}
					//次像素估計
					if (ptMaxLoc.x == 0 || ptMaxLoc.y == 0 || ptMaxLoc.x == matResult.cols - 1 || ptMaxLoc.y == matResult.rows - 1)
						vecNewMatchParameter[j].bPosOnBorder = true;
					if (!vecNewMatchParameter[j].bPosOnBorder)
					{
						for (int y = -1; y <= 1; y++)
							for (int x = -1; x <= 1; x++)
								vecNewMatchParameter[j].vecResult[x + 1][y + 1] = matResult.at<float> (ptMaxLoc + cv::Point (x, y));
					}
					//次像素估計
				}
				if (vecNewMatchParameter[iMaxScoreIndex].dMatchScore < vecLayerScore[iLayer])
					break;
				//次像素估計
				if (bSubPixelEstimation 
					&& iLayer == 0 
					&& (!vecNewMatchParameter[iMaxScoreIndex].bPosOnBorder) 
					&& iMaxScoreIndex != 0 
					&& iMaxScoreIndex != 2)
				{
					double dNewX = 0, dNewY = 0, dNewAngle = 0;
					subPixEstimation ( &vecNewMatchParameter, &dNewX, &dNewY, &dNewAngle, dAngleStep, iMaxScoreIndex);
					vecNewMatchParameter[iMaxScoreIndex].pt = cv::Point2d (dNewX, dNewY);
					vecNewMatchParameter[iMaxScoreIndex].dMatchAngle = dNewAngle;
				}
				//次像素估計

				double dNewMatchAngle = vecNewMatchParameter[iMaxScoreIndex].dMatchAngle;

				//讓坐標系回到旋轉時(GetRotatedROI)的(0, 0)
				cv::Point2f ptPaddingLT = ptRotatePt2f (ptLT * 2, ptSrcCenter, dNewMatchAngle * D2R) - cv::Point2f (3, 3);
				cv::Point2f pt (vecNewMatchParameter[iMaxScoreIndex].pt.x + ptPaddingLT.x, vecNewMatchParameter[iMaxScoreIndex].pt.y + ptPaddingLT.y);
				//再旋轉
				pt = ptRotatePt2f (pt, ptSrcCenter, -dNewMatchAngle * D2R);

				if (iLayer == iStopLayer)
				{
					vecNewMatchParameter[iMaxScoreIndex].pt = pt * (iStopLayer == 0 ? 1 : 2);
					vecAllResult.push_back (vecNewMatchParameter[iMaxScoreIndex]);
				}
				else
				{
					//更新MatchAngle ptLT
					vecMatchParameter[i].dMatchAngle = dNewMatchAngle;
					vecMatchParameter[i].dAngleStart = vecMatchParameter[i].dMatchAngle - dAngleStep / 2;
					vecMatchParameter[i].dAngleEnd = vecMatchParameter[i].dMatchAngle + dAngleStep / 2;
					ptLT = pt;
				}
			}

		}
	}
    // 过滤结果
    filterWithScore(&vecAllResult, m_dScore);
    
	//最後濾掉重疊
	iDstW = pTemplData->vecPyramid[iStopLayer].cols * (iStopLayer == 0 ? 1 : 2);
	iDstH = pTemplData->vecPyramid[iStopLayer].rows * (iStopLayer == 0 ? 1 : 2);

    
	for (int i = 0; i < (int)vecAllResult.size (); i++)
	{
		cv::Point2f ptLT, ptRT, ptRB, ptLB;
		double dRAngle = -vecAllResult[i].dMatchAngle * D2R;
		ptLT = vecAllResult[i].pt;
		ptRT = cv::Point2f (ptLT.x + iDstW * (float)cos (dRAngle), ptLT.y - iDstW * (float)sin (dRAngle));
		ptLB = cv::Point2f (ptLT.x + iDstH * (float)sin (dRAngle), ptLT.y + iDstH * (float)cos (dRAngle));
		ptRB = cv::Point2f (ptRT.x + iDstH * (float)sin (dRAngle), ptRT.y + iDstH * (float)cos (dRAngle));
		//紀錄旋轉矩形
		vecAllResult[i].rectR = cv::RotatedRect(ptLT, ptRT, ptRB);
	}
	filterWithRotatedRect (&vecAllResult, cv::TM_CCOEFF_NORMED, m_dMaxOverlap);
	//最後濾掉重疊

	//根據分數排序
	std::sort (vecAllResult.begin (), vecAllResult.end (), compareScoreBig2Small);
	
	iMatchSize = (int)vecAllResult.size ();
	if (vecAllResult.size () == 0)
		return std::vector<s_SingleTargetMatch>();
	int iW = pTemplData->vecPyramid[0].cols, iH = pTemplData->vecPyramid[0].rows;

    // 记录执行时间
    auto endTime = std::chrono::high_resolution_clock::now();
    m_dLastExecutionTime = std::chrono::duration<double>(endTime - startTime).count();
    
    // 转换结果为 s_SingleTargetMatch 格式
    std::vector<s_SingleTargetMatch> results;
    for (const auto& matchParam : vecAllResult) {
        s_SingleTargetMatch result;
        
        // 计算四个角点坐标
        double dRAngle = -matchParam.dMatchAngle * D2R;
        cv::Point2f ptLT = matchParam.pt;
        cv::Point2f ptRT = cv::Point2f(ptLT.x + iW * (float)cos(dRAngle), ptLT.y - iW * (float)sin(dRAngle));
        cv::Point2f ptLB = cv::Point2f(ptLT.x + iH * (float)sin(dRAngle), ptLT.y + iH * (float)cos(dRAngle));
        cv::Point2f ptRB = cv::Point2f(ptRT.x + iH * (float)sin(dRAngle), ptRT.y + iH * (float)cos(dRAngle));
        
        // 计算中心点
        cv::Point2f ptCenter = cv::Point2f((ptLT.x + ptRT.x + ptLB.x + ptRB.x) / 4.0f, 
                                           (ptLT.y + ptRT.y + ptLB.y + ptRB.y) / 4.0f);
        
        // 设置结果
        result.ptLT = ptLT;
        result.ptRT = ptRT;
        result.ptLB = ptLB;
        result.ptRB = ptRB;
        result.ptCenter = ptCenter;
        result.dMatchedAngle = matchParam.dMatchAngle;
        result.dMatchScore = matchParam.dMatchScore;
        
        results.push_back(result);
    }
    
    std::cout << "匹配完成，找到 " << results.size() << " 个结果，执行时间: " << m_dLastExecutionTime << " 秒" << std::endl;
    
    return results;
}

void TemplateMatcher::clearPattern()
{
    m_TemplData.clear();
    m_TemplData.bIsPatternLearned = false;
}

int TemplateMatcher::getTopLayer(const cv::Mat* matTempl, int iMinDstLength)
{
    int iTopLayer = 0;
    int iMinReduceArea = iMinDstLength * iMinDstLength;
    int iArea = matTempl->cols * matTempl->rows;
    while (iArea > iMinReduceArea) {
        iArea /= 4;
        iTopLayer++;
    }
    return iTopLayer;
}

// 基於SSE的字節數據的乘法。
// <param name="Kernel">需要卷積的核矩陣。 </param>
// <param name="Conv">卷積矩陣。 </param>
// <param name="Length">矩陣所有元素的長度。 </param>
inline int IM_Conv_SIMD (unsigned char* pCharKernel, unsigned char *pCharConv, int iLength)
{
	const int iBlockSize = 16, Block = iLength / iBlockSize;
	__m128i SumV = _mm_setzero_si128 ();
	__m128i Zero = _mm_setzero_si128 ();
	for (int Y = 0; Y < Block * iBlockSize; Y += iBlockSize)
	{
		__m128i SrcK = _mm_loadu_si128 ((__m128i*)(pCharKernel + Y));
		__m128i SrcC = _mm_loadu_si128 ((__m128i*)(pCharConv + Y));
		__m128i SrcK_L = _mm_unpacklo_epi8 (SrcK, Zero);
		__m128i SrcK_H = _mm_unpackhi_epi8 (SrcK, Zero);
		__m128i SrcC_L = _mm_unpacklo_epi8 (SrcC, Zero);
		__m128i SrcC_H = _mm_unpackhi_epi8 (SrcC, Zero);
		__m128i SumT = _mm_add_epi32 (_mm_madd_epi16 (SrcK_L, SrcC_L), _mm_madd_epi16 (SrcK_H, SrcC_H));
		SumV = _mm_add_epi32 (SumV, SumT);
	}
	int Sum = _mm_hsum_epi32 (SumV);
	for (int Y = Block * iBlockSize; Y < iLength; Y++)
	{
		Sum += pCharKernel[Y] * pCharConv[Y];
	}
	return Sum;
}

void TemplateMatcher::MatchTemplate(cv::Mat& matSrc, s_TemplData* pTemplData, cv::Mat& matResult, int iLayer, bool bUseSIMD)
{
	if (m_bUseSIMD && bUseSIMD)
	{
		//From ImageShop
		matResult.create (matSrc.rows - pTemplData->vecPyramid[iLayer].rows + 1,
			matSrc.cols - pTemplData->vecPyramid[iLayer].cols + 1, CV_32FC1);
		matResult.setTo (0);
		cv::Mat& matTemplate = pTemplData->vecPyramid[iLayer];

		int  t_r_end = matTemplate.rows, t_r = 0;
		for (int r = 0; r < matResult.rows; r++)
		{
			float* r_matResult = matResult.ptr<float> (r);
			uchar* r_source = matSrc.ptr<uchar> (r);
			uchar* r_template, *r_sub_source;
			for (int c = 0; c < matResult.cols; ++c, ++r_matResult, ++r_source)
			{
				r_template = matTemplate.ptr<uchar> ();
				r_sub_source = r_source;
				for (t_r = 0; t_r < t_r_end; ++t_r, r_sub_source += matSrc.cols, r_template += matTemplate.cols)
				{
					*r_matResult = *r_matResult + IM_Conv_SIMD (r_template, r_sub_source, matTemplate.cols);
				}
			}
		}
		//From ImageShop
	}
	else
        cv::matchTemplate (matSrc, pTemplData->vecPyramid[iLayer], matResult, cv::TM_CCORR);
	//保存结果图片
	// cv::imwrite("result.png", matResult);
	// cv::imwrite("src.png", matSrc);
	// cv::imwrite("template.png", pTemplData->vecPyramid[iLayer]);
	/*Mat diff;
	absdiff(matResult, matResult, diff);
	double dMaxValue;
	minMaxLoc(diff, 0, &dMaxValue, 0,0);*/
	CCOEFF_Denominator (matSrc, pTemplData, matResult, iLayer);
}

void TemplateMatcher::CCOEFF_Denominator(cv::Mat& matSrc, s_TemplData* pTemplData, cv::Mat& matResult, int iLayer)
{
    if (pTemplData->vecResultEqual1[iLayer])
	{
		matResult = cv::Scalar::all (1);
		return;
	}
	double *q0 = 0, *q1 = 0, *q2 = 0, *q3 = 0;

	cv::Mat sum, sqsum;
	cv::integral (matSrc, sum, sqsum, CV_64F);

	q0 = (double*)sqsum.data;
	q1 = q0 + pTemplData->vecPyramid[iLayer].cols;
	q2 = (double*)(sqsum.data + pTemplData->vecPyramid[iLayer].rows * sqsum.step);
	q3 = q2 + pTemplData->vecPyramid[iLayer].cols;

	double* p0 = (double*)sum.data;
	double* p1 = p0 + pTemplData->vecPyramid[iLayer].cols;
	double* p2 = (double*)(sum.data + pTemplData->vecPyramid[iLayer].rows*sum.step);
	double* p3 = p2 + pTemplData->vecPyramid[iLayer].cols;

	int sumstep = sum.data ? (int)(sum.step / sizeof (double)) : 0;
	int sqstep = sqsum.data ? (int)(sqsum.step / sizeof (double)) : 0;

	//
	double dTemplMean0 = pTemplData->vecTemplMean[iLayer][0];
	double dTemplNorm = pTemplData->vecTemplNorm[iLayer];
	double dInvArea = pTemplData->vecInvArea[iLayer];
	//

	int i, j;
	for (i = 0; i < matResult.rows; i++)
	{
		float* rrow = matResult.ptr<float> (i);
		int idx = i * sumstep;
		int idx2 = i * sqstep;

		for (j = 0; j < matResult.cols; j += 1, idx += 1, idx2 += 1)
		{
			double num = rrow[j], t;
			double wndMean2 = 0, wndSum2 = 0;

			t = p0[idx] - p1[idx] - p2[idx] + p3[idx];
			wndMean2 += t * t;
			num -= t * dTemplMean0;
			wndMean2 *= dInvArea;


			t = q0[idx2] - q1[idx2] - q2[idx2] + q3[idx2];
			wndSum2 += t;


			//t = std::sqrt (MAX (wndSum2 - wndMean2, 0)) * dTemplNorm;

			double diff2 = std::max (wndSum2 - wndMean2, 0.0);
			if (diff2 <= std::min (0.5, 10 * FLT_EPSILON * wndSum2))
				t = 0; // avoid rounding errors
			else
				t = std::sqrt (diff2)*dTemplNorm;

			if (fabs (num) < t)
				num /= t;
			else if (fabs (num) < t * 1.125)
				num = num > 0 ? 1 : -1;
			else
				num = 0;

			rrow[j] = (float)num;
		}
	}
}

cv::Size TemplateMatcher::getBestRotationSize(cv::Size sizeSrc, cv::Size sizeDst, double dRAngle)
{
    double dRAngle_radian = dRAngle * D2R;
	cv::Point ptLT (0, 0), ptLB (0, sizeSrc.height - 1), ptRB (sizeSrc.width - 1, sizeSrc.height - 1), ptRT (sizeSrc.width - 1, 0);
	cv::Point2f ptCenter ((sizeSrc.width - 1) / 2.0f, (sizeSrc.height - 1) / 2.0f);
	cv::Point2f ptLT_R = ptRotatePt2f (cv::Point2f (ptLT), ptCenter, dRAngle_radian);
	cv::Point2f ptLB_R = ptRotatePt2f (cv::Point2f (ptLB), ptCenter, dRAngle_radian);
	cv::Point2f ptRB_R = ptRotatePt2f (cv::Point2f (ptRB), ptCenter, dRAngle_radian);
	cv::Point2f ptRT_R = ptRotatePt2f (cv::Point2f (ptRT), ptCenter, dRAngle_radian);

	float fTopY = std::max (std::max (ptLT_R.y, ptLB_R.y), std::max (ptRB_R.y, ptRT_R.y));
	float fBottomY = std::min (std::min (ptLT_R.y, ptLB_R.y), std::min (ptRB_R.y, ptRT_R.y));
	float fRightX = std::max (std::max (ptLT_R.x, ptLB_R.x), std::max (ptRB_R.x, ptRT_R.x));
	float fLeftX = std::min (std::min (ptLT_R.x, ptLB_R.x), std::min (ptRB_R.x, ptRT_R.x));

	if (dRAngle > 360)
		dRAngle -= 360;
	else if (dRAngle < 0)
		dRAngle += 360;

	if (fabs (fabs (dRAngle) - 90) < VISION_TOLERANCE || fabs (fabs (dRAngle) - 270) < VISION_TOLERANCE)
	{
		return cv::Size (sizeSrc.height, sizeSrc.width);
	}
	else if (fabs (dRAngle) < VISION_TOLERANCE || fabs (fabs (dRAngle) - 180) < VISION_TOLERANCE)
	{
		return sizeSrc;
	}
	
	double dAngle = dRAngle;

	if (dAngle > 0 && dAngle < 90)
	{
		;
	}
	else if (dAngle > 90 && dAngle < 180)
	{
		dAngle -= 90;
	}
	else if (dAngle > 180 && dAngle < 270)
	{
		dAngle -= 180;
	}
	else if (dAngle > 270 && dAngle < 360)
	{
		dAngle -= 270;
	}
	else//Debug
	{
		//AfxMessageBox (L"Unkown");
        qDebug() << "Unkown";
	}

	float fH1 = sizeDst.width * sin (dAngle * D2R) * cos (dAngle * D2R);
	float fH2 = sizeDst.height * sin (dAngle * D2R) * cos (dAngle * D2R);

	int iHalfHeight = (int)ceil (fTopY - ptCenter.y - fH1);
	int iHalfWidth = (int)ceil (fRightX - ptCenter.x - fH2);
	
	cv::Size sizeRet (iHalfWidth * 2, iHalfHeight * 2);

	bool bWrongSize = (sizeDst.width < sizeRet.width && sizeDst.height > sizeRet.height)
		|| (sizeDst.width > sizeRet.width && sizeDst.height < sizeRet.height
			|| sizeDst.area () > sizeRet.area ());
	if (bWrongSize)
		sizeRet = cv::Size (int (fRightX - fLeftX + 0.5), int (fTopY - fBottomY + 0.5));

	return sizeRet;
}

cv::Point2f TemplateMatcher::ptRotatePt2f(cv::Point2f ptInput, cv::Point2f ptOrg, double dAngle)
{
    double dWidth = ptOrg.x * 2;
	double dHeight = ptOrg.y * 2;
	double dY1 = dHeight - ptInput.y, dY2 = dHeight - ptOrg.y;

	double dX = (ptInput.x - ptOrg.x) * cos (dAngle) - (dY1 - ptOrg.y) * sin (dAngle) + ptOrg.x;
	double dY = (ptInput.x - ptOrg.x) * sin (dAngle) + (dY1 - ptOrg.y) * cos (dAngle) + dY2;

	dY = -dY + dHeight;
	return cv::Point2f ((float)dX, (float)dY);
}

void TemplateMatcher::filterWithScore(std::vector<s_MatchParameter>* vec, double dScore)
{
    std::sort (vec->begin (), vec->end (), compareScoreBig2Small);
	int iSize = vec->size (), iIndexDelete = iSize + 1;
	for (int i = 0; i < iSize; i++)
	{
		if ((*vec)[i].dMatchScore < dScore)
		{
			iIndexDelete = i;
			break;
		}
	}
	if (iIndexDelete == iSize + 1)//沒有任何元素小於dScore
		return;
	vec->erase (vec->begin () + iIndexDelete, vec->end ());
	return;
}

bool TemplateMatcher::subPixEstimation(std::vector<s_MatchParameter>* vec, double* dX, double* dY, double* dAngle, double dAngleStep, int iMaxScoreIndex)
{
    cv::Mat matA (27, 10, CV_64F);
	cv::Mat matZ (10, 1, CV_64F);
	cv::Mat matS (27, 1, CV_64F);

	double dX_maxScore = (*vec)[iMaxScoreIndex].pt.x;
	double dY_maxScore = (*vec)[iMaxScoreIndex].pt.y;
	double dTheata_maxScore = (*vec)[iMaxScoreIndex].dMatchAngle;
	int iRow = 0;
	/*for (int x = -1; x <= 1; x++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int theta = 0; theta <= 2; theta++)
			{*/
	for (int theta = 0; theta <= 2; theta++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int x = -1; x <= 1; x++)
			{
				//xx yy tt xy xt yt x y t 1
				//0  1  2  3  4  5  6 7 8 9
				double dX = dX_maxScore + x;
				double dY = dY_maxScore + y;
				//double dT = (*vec)[theta].dMatchAngle + (theta - 1) * dAngleStep;
				double dT = (dTheata_maxScore + (theta - 1) * dAngleStep) * D2R;
				matA.at<double> (iRow, 0) = dX * dX;
				matA.at<double> (iRow, 1) = dY * dY;
				matA.at<double> (iRow, 2) = dT * dT;
				matA.at<double> (iRow, 3) = dX * dY;
				matA.at<double> (iRow, 4) = dX * dT;
				matA.at<double> (iRow, 5) = dY * dT;
				matA.at<double> (iRow, 6) = dX;
				matA.at<double> (iRow, 7) = dY;
				matA.at<double> (iRow, 8) = dT;
				matA.at<double> (iRow, 9) = 1.0;
				matS.at<double> (iRow, 0) = (*vec)[iMaxScoreIndex + (theta - 1)].vecResult[x + 1][y + 1];
				iRow++;
#ifdef _DEBUG
				/*string str = format ("%.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f, %.6f", dValueA[0], dValueA[1], dValueA[2], dValueA[3], dValueA[4], dValueA[5], dValueA[6], dValueA[7], dValueA[8], dValueA[9]);
				fileA <<  str << endl;
				str = format ("%.6f", dValueS[iRow]);
				fileS << str << endl;*/
#endif
			}
		}
	}
	//求解Z矩陣，得到k0~k9
	//[ x* ] = [ 2k0 k3 k4 ]-1 [ -k6 ]
	//| y* | = | k3 2k1 k5 |   | -k7 |
	//[ t* ] = [ k4 k5 2k2 ]   [ -k8 ]
	
	//solve (matA, matS, matZ, DECOMP_SVD);
	matZ = (matA.t () * matA).inv () * matA.t ()* matS;
	cv::Mat matZ_t;
	cv::transpose (matZ, matZ_t);
	double* dZ = matZ_t.ptr<double> (0);
	cv::Mat matK1 = (cv::Mat_<double> (3, 3) << 
		(2 * dZ[0]), dZ[3], dZ[4], 
		dZ[3], (2 * dZ[1]), dZ[5], 
		dZ[4], dZ[5], (2 * dZ[2]));
	cv::Mat matK2 = (cv::Mat_<double> (3, 1) << -dZ[6], -dZ[7], -dZ[8]);
	cv::Mat matDelta = matK1.inv () * matK2;

	*dX = matDelta.at<double> (0, 0);
	*dY = matDelta.at<double> (1, 0);
	*dAngle = matDelta.at<double> (2, 0) * R2D;
	return true;
}

void TemplateMatcher::getRotatedROI(cv::Mat& matSrc, cv::Size size, cv::Point2f ptLT, double dAngle, cv::Mat& matROI)
{
	double dAngle_radian = dAngle * D2R;
	cv::Point2f ptC ((matSrc.cols - 1) / 2.0f, (matSrc.rows - 1) / 2.0f);
	cv::Point2f ptLT_rotate = ptRotatePt2f (ptLT, ptC, dAngle_radian);
	cv::Size sizePadding (size.width + 6, size.height + 6);


	cv::Mat rMat = cv::getRotationMatrix2D (ptC, dAngle, 1);
	rMat.at<double> (0, 2) -= ptLT_rotate.x - 3;
	rMat.at<double> (1, 2) -= ptLT_rotate.y - 3;
	//平移旋轉矩陣(0, 2) (1, 2)的減，為旋轉後的圖形偏移，-= ptLT_rotate.x - 3 代表旋轉後的圖形往-X方向移動ptLT_rotate.x - 3
	//Debug
	
	//Debug
	warpAffine (matSrc, matROI, rMat, sizePadding);
}


void TemplateMatcher::sortPtWithCenter(std::vector<cv::Point2f>& vecSort)
{
	int iSize = (int)vecSort.size ();
	cv::Point2f ptCenter;
	for (int i = 0; i < iSize; i++)
		ptCenter += vecSort[i];
	ptCenter /= iSize;

	cv::Point2f vecX (1, 0);

	std::vector<std::pair<cv::Point2f, double>> vecPtAngle (iSize);
	for (int i = 0; i < iSize; i++)
	{
		vecPtAngle[i].first = vecSort[i];//pt
		cv::Point2f vec1 (vecSort[i].x - ptCenter.x, vecSort[i].y - ptCenter.y);
		float fNormVec1 = vec1.x * vec1.x + vec1.y * vec1.y;
		float fDot = vec1.x;

		if (vec1.y < 0)//若點在中心的上方
		{
			vecPtAngle[i].second = acos (fDot / fNormVec1) * R2D;
		}
		else if (vec1.y > 0)//下方
		{
			vecPtAngle[i].second = 360 - acos (fDot / fNormVec1) * R2D;
		}
		else//點與中心在相同Y
		{
			if (vec1.x - ptCenter.x > 0)
				vecPtAngle[i].second = 0;
			else
				vecPtAngle[i].second = 180;
		}

	}
	std::sort (vecPtAngle.begin (), vecPtAngle.end (), comparePtWithAngle);
	for (int i = 0; i < iSize; i++)
		vecSort[i] = vecPtAngle[i].first;
}

void TemplateMatcher::filterWithRotatedRect(std::vector<s_MatchParameter>* vec, int iMethod, double dMaxOverLap)
{
    if (vec->empty()) return;
    
    int iMatchSize = (int)vec->size ();
	cv::RotatedRect rect1, rect2;
	for (int i = 0; i < iMatchSize - 1; i++)
	{
		if (vec->at (i).bDelete)
			continue;
		for (int j = i + 1; j < iMatchSize; j++)
		{
			if (vec->at (j).bDelete)
				continue;
			rect1 = vec->at (i).rectR;
			rect2 = vec->at (j).rectR;
			std::vector<cv::Point2f> vecInterSec;
			int iInterSecType = rotatedRectangleIntersection (rect1, rect2, vecInterSec);
			if (iInterSecType == cv::INTERSECT_NONE)//無交集
				continue;
			else if (iInterSecType == cv::INTERSECT_FULL) //一個矩形包覆另一個
			{
				int iDeleteIndex;
				if (iMethod == cv::TM_SQDIFF)
					iDeleteIndex = (vec->at (i).dMatchScore <= vec->at (j).dMatchScore) ? j : i;
				else
					iDeleteIndex = (vec->at (i).dMatchScore >= vec->at (j).dMatchScore) ? j : i;
				vec->at (iDeleteIndex).bDelete = true;
			}
			else//交點 > 0
			{
				if (vecInterSec.size () < 3)//一個或兩個交點
					continue;
				else
				{
					int iDeleteIndex;
					//求面積與交疊比例
					sortPtWithCenter (vecInterSec);
					double dArea = contourArea (vecInterSec);
					double dRatio = dArea / rect1.size.area ();
					//若大於最大交疊比例，選分數高的
					if (dRatio > dMaxOverLap)
					{
						if (iMethod == cv::TM_SQDIFF)
							iDeleteIndex = (vec->at (i).dMatchScore <= vec->at (j).dMatchScore) ? j : i;
						else
							iDeleteIndex = (vec->at (i).dMatchScore >= vec->at (j).dMatchScore) ? j : i;
						vec->at (iDeleteIndex).bDelete = true;
					}
				}
			}
		}
	}
	std::vector<s_MatchParameter>::iterator it;
	for (it = vec->begin (); it != vec->end ();)
	{
		if ((*it).bDelete)
			it = vec->erase (it);
		else
			++it;
	}
}

cv::Point TemplateMatcher::getNextMaxLoc(cv::Mat& matResult, cv::Point ptMaxLoc, cv::Size sizeTemplate, double& dMaxValue, double dMaxOverlap)
{
    int iStartX = ptMaxLoc.x - sizeTemplate.width * (1 - dMaxOverlap);
	int iStartY = ptMaxLoc.y - sizeTemplate.height * (1 - dMaxOverlap);
	//塗黑
	cv::rectangle (matResult, cv::Rect (iStartX, iStartY, 2 * sizeTemplate.width * (1- dMaxOverlap), 2 * sizeTemplate.height * (1- dMaxOverlap)), cv::Scalar (-1), cv::FILLED);
	//得到下一個最大值
	cv::Point ptNewMaxLoc;
	cv::minMaxLoc (matResult, 0, &dMaxValue, 0, &ptNewMaxLoc);
	return ptNewMaxLoc;
}

cv::Point TemplateMatcher::getNextMaxLoc(cv::Mat& matResult, cv::Point ptMaxLoc, cv::Size sizeTemplate, double& dMaxValue, double dMaxOverlap, s_BlockMax& blockMax)
{
    //比對到的區域需考慮重疊比例
	int iStartX = int (ptMaxLoc.x - sizeTemplate.width * (1 - dMaxOverlap));
	int iStartY = int (ptMaxLoc.y - sizeTemplate.height * (1 - dMaxOverlap));
	cv::Rect rectIgnore (iStartX, iStartY, int (2 * sizeTemplate.width * (1 - dMaxOverlap))
		, int (2 * sizeTemplate.height * (1 - dMaxOverlap)));
	//塗黑
	cv::rectangle (matResult, rectIgnore , cv::Scalar (-1), cv::FILLED);
	blockMax.UpdateMax (rectIgnore);
	cv::Point ptReturn;
	blockMax.GetMaxValueLoc (dMaxValue, ptReturn);
	return ptReturn;
}
