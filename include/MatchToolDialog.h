#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QScrollBar>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QTimer>
#include <QElapsedTimer>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPainter>

#include "TemplateMatcher.h"
#include "DataStructures.h"

// 前向声明
namespace Ui {
    class MatchToolDialog;
}

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QGraphicsView;
class QTableWidget;
class QLabel;
class QPushButton;
class QCheckBox;
class QSpinBox;
class QDoubleSpinBox;
class QComboBox;
class QStatusBar;
class QMenuBar;
QT_END_NAMESPACE

// Qt版本的匹配工具对话框
class MatchToolDialog : public QDialog
{
    Q_OBJECT

public:
    explicit MatchToolDialog(QWidget *parent = nullptr);
    ~MatchToolDialog();
    
    // 公共方法
    void loadSourceImage(const QString& filePath);
    void loadTemplateImage(const QString& filePath);

protected:
    // 拖拽事件
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    
    // 鼠标滚轮事件
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    // UI事件处理
    void onLoadSrcButtonClicked();
    void onLoadDstButtonClicked();
    void onExecuteButtonClicked();

private:
    // UI初始化
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();
    
    // 图像处理
    void refreshSourceView();
    void refreshTemplateView();
    void displayResults();
    void highlightSelectedMatch(int index);
    
    // 文件操作
    QString openImageFile(const QString& title, const QString& filter);
    void saveResultsToFile(const QString& filePath);
    
    // 工具函数
    void updateStatusBar(const QString& message);
    void showMessage(const QString& title, const QString& message);
    void updateExecutionTime(double timeMs);
    void clearResults();
    
    // 图像转换
    QImage matToQImage(const cv::Mat& mat);
    QPixmap matToQPixmap(const cv::Mat& mat);
    
    // 结果显示
    void refreshSourceViewWithResults();
    
    // 清除结果
        void onClearButtonClicked();
    
    // 绘制辅助函数
    void drawDashLine(cv::Mat& matDraw, cv::Point2d ptStart, cv::Point2d ptEnd, cv::Scalar color1, cv::Scalar color2);
    void drawMarkCross(cv::Mat& matDraw, double x, double y, int length, cv::Scalar color, int thickness);
    
    // 矩形绘制相关
    void setupTemplateViewMouseEvents();
    void onTemplateViewMousePress(QMouseEvent* event);
    void onTemplateViewMouseMove(QMouseEvent* event);
    void onTemplateViewMouseRelease(QMouseEvent* event);
    void drawUserRectOnTemplate();
    void drawUserRectOnResults();
    void addCornerMarkers(const cv::Rect& rect);
    void addSizeLabel(const cv::Rect& rect);
    
    // 多边形选择相关
    void setupPolygonSelection();
    void onTemplateViewRightClick(QMouseEvent* event);
    void startPolygonSelection();
    void finishPolygonSelection();
    void drawPolygonOnTemplate();
    void drawPolygonOnResults();
    void drawUserPolygonOnResults(const s_SingleTargetMatch& result, cv::Mat& displayImage);
    void addPolygonPoint(const QPoint& pos);
    void clearPolygon();
    
    cv::Rect transformRectToResult(const cv::Rect& templateRect, const s_SingleTargetMatch& match);
    void clearUserRect();
    
    // 多边形变换相关
    std::vector<cv::Point2f> transformPolygonToResult(const std::vector<cv::Point2f>& templatePolygon, const s_SingleTargetMatch& match);
    void clearUserPolygon();
    
    // 缩放相关函数
    void applyZoom();
    void applyZoomAtMousePosition(const QPoint& mousePos);
    
    // 模板视图缩放相关函数
    void applyTemplateZoom();
    void applyTemplateZoomAtMousePosition(const QPoint& mousePos);
    
    // 状态栏更新函数
    void updateSourceImageLabel();
    void updateTemplateImageLabel();
    void updateExecutionTimeLabel(double timeMs);
    
    // 设置保存和加载函数
    void saveSettings();
    void loadSettings();
    void saveImagePaths();
    void loadImagePaths();
    void onParameterChanged();

 

private:
    // UI指针
    Ui::MatchToolDialog* ui;
    
    // 核心算法
    TemplateMatcher m_matcher;
    
    // 图像数据
    cv::Mat m_sourceImage;
    cv::Mat m_templateImage;
    std::vector<s_SingleTargetMatch> m_matchResults;
    
    // 图像场景
    QGraphicsScene* m_sourceScene;
    QGraphicsScene* m_templateScene;
    
    // 状态
    bool m_sourceImageLoaded;
    bool m_templateImageLoaded;
    bool m_resultsAvailable;
    double m_lastExecutionTime;
    
    // 文件对话框记忆
    QString m_lastDirectory;
    
    // 设置相关
    QString m_lastSourceImagePath;
    QString m_lastTemplateImagePath;
    
    // 缩放相关变量（模拟MFC）
    double m_dSrcScale;
    double m_dNewScale;
    bool m_bShowResult;
    int m_iScaleTimes;
    
    // 鼠标滚轮缩放相关
    double m_currentScale;
    double m_minScale;
    double m_maxScale;
    
    // 模板视图缩放相关变量
    double m_templateScale;
    double m_templateMinScale;
    double m_templateMaxScale;
    
    // 状态栏标签
    QLabel* m_sourceImageLabel;
    QLabel* m_templateImageLabel;
    QLabel* m_executionTimeLabel;
    
    // 定时器
    QElapsedTimer m_executionTimer;
    
    // 矩形绘制相关变量
    bool m_isDrawingRect;
    QPoint m_rectStartPoint;
    QPoint m_rectEndPoint;
    cv::Rect m_userDefinedRect;
    bool m_hasUserRect;
    
    // 多边形选择相关变量
    bool m_isSelectingPolygon;
    std::vector<cv::Point2f> m_polygonPoints;
    bool m_hasUserPolygon;
}; 