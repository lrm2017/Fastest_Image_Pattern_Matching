#include "MatchToolDialog.h"
#include "ui_MatchToolDialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <QApplication>
#include <chrono>
#include <QSettings>

MatchToolDialog::MatchToolDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::MatchToolDialog)
    , m_sourceScene(nullptr)
    , m_templateScene(nullptr)
    , m_sourceImageLoaded(false)
    , m_templateImageLoaded(false)
    , m_resultsAvailable(false)
    , m_lastExecutionTime(0.0)
    , m_lastDirectory(QDir::homePath())
    , m_currentScale(1.0)
    , m_minScale(0.1)
    , m_maxScale(10.0)
    , m_templateScale(1.0)
    , m_templateMinScale(0.1)
    , m_templateMaxScale(10.0)
    , m_sourceImageLabel(nullptr)
    , m_templateImageLabel(nullptr)
    , m_executionTimeLabel(nullptr)
    , m_lastSourceImagePath("")
    , m_lastTemplateImagePath("")
    , m_isDrawingRect(false)
    , m_hasUserRect(false)
    , m_isSelectingPolygon(false)
    , m_hasUserPolygon(false)
{
    ui->setupUi(this);
    setupUI();
    setupConnections();
}

MatchToolDialog::~MatchToolDialog()
{
    // 保存设置
    saveSettings();
    delete ui;
}

void MatchToolDialog::setupUI()
{
    // 基本UI设置
    setAcceptDrops(true);
    
    // 设置表格列标题
    ui->resultsTable->setHorizontalHeaderLabels({
        "索引", "分数", "角度", "X坐标", "Y坐标"
    });
    
    // 初始化图像场景
    if (!m_sourceScene) {
        m_sourceScene = new QGraphicsScene(this);
    }
    if (!m_templateScene) {
        m_templateScene = new QGraphicsScene(this);
    }
    
    ui->sourceView->setScene(m_sourceScene);
    ui->templateView->setScene(m_templateScene);
    
    // 设置渲染选项
    ui->sourceView->setRenderHint(QPainter::Antialiasing);
    ui->sourceView->setRenderHint(QPainter::SmoothPixmapTransform);
    ui->templateView->setRenderHint(QPainter::Antialiasing);
    ui->templateView->setRenderHint(QPainter::SmoothPixmapTransform);
    
    // 设置滚动条策略
    ui->sourceView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->sourceView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->templateView->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    ui->templateView->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    
    // 设置源视图的交互功能
    ui->sourceView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->sourceView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->sourceView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    ui->sourceView->setInteractive(true);
    
    // 设置模板视图的交互功能
    ui->templateView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->templateView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    ui->templateView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);
    ui->templateView->setInteractive(true);
    
    // 安装事件过滤器以处理鼠标滚轮缩放和矩形绘制
    ui->sourceView->viewport()->installEventFilter(this);
    ui->templateView->viewport()->installEventFilter(this);
    
    // 设置源视图的右键菜单
    ui->sourceView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->sourceView, &QGraphicsView::customContextMenuRequested, this, [this](const QPoint& pos) {
        if (m_hasUserRect) {
            QMenu contextMenu(this);
            QAction* clearAction = contextMenu.addAction("清除用户矩形");
            QAction* selectedAction = contextMenu.exec(ui->sourceView->mapToGlobal(pos));
            
            if (selectedAction == clearAction) {
                clearUserRect();
            }
        }
    });
    
    // 设置模板视图的右键菜单
    ui->templateView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->templateView, &QGraphicsView::customContextMenuRequested, this, [this](const QPoint& pos) {
        QMenu contextMenu(this);
        
        if (m_hasUserRect) {
            QAction* clearRectAction = contextMenu.addAction("清除用户矩形");
            contextMenu.addSeparator();
        }
        
        if (m_hasUserPolygon) {
            QAction* clearPolygonAction = contextMenu.addAction("清除用户多边形");
            contextMenu.addSeparator();
        }
        
        QAction* startPolygonAction = contextMenu.addAction("开始多边形选择");
        
        QAction* selectedAction = contextMenu.exec(ui->templateView->mapToGlobal(pos));
        
        if (selectedAction) {
            if (selectedAction->text() == "清除用户矩形") {
                clearUserRect();
            } else if (selectedAction->text() == "清除用户多边形") {
                clearPolygon();
            } else if (selectedAction->text() == "开始多边形选择") {
                startPolygonSelection();
            }
        }
    });
    
    // 设置模板视图的鼠标事件（已在上方设置）
    
    // 设置状态栏样式
    ui->statusBar->setStyleSheet("QStatusBar::item { border: none; }");
    
    // 创建状态栏标签
    m_sourceImageLabel = new QLabel("源图像: 未加载", this);
    m_templateImageLabel = new QLabel("模板图像: 未加载", this);
    m_executionTimeLabel = new QLabel("执行时间: 0.000s", this);
    
    // 设置标签样式
    m_sourceImageLabel->setMinimumWidth(200);
    m_templateImageLabel->setMinimumWidth(200);
    m_executionTimeLabel->setMinimumWidth(150);
    
    // 添加标签到状态栏
    ui->statusBar->addWidget(m_sourceImageLabel);
    ui->statusBar->addWidget(m_templateImageLabel);
    ui->statusBar->addPermanentWidget(m_executionTimeLabel);
    
    // 加载设置
    loadSettings();
}

void MatchToolDialog::setupTemplateViewMouseEvents()
{
    // 设置模板视图的鼠标事件
    ui->templateView->viewport()->installEventFilter(this);
    
    // 设置模板视图为可交互模式
    ui->templateView->setDragMode(QGraphicsView::NoDrag);
    ui->templateView->setInteractive(true);
    
    // 设置右键菜单
    ui->templateView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->templateView, &QGraphicsView::customContextMenuRequested, this, [this](const QPoint& pos) {
        if (m_hasUserRect) {
            QMenu contextMenu(this);
            QAction* clearAction = contextMenu.addAction("清除用户矩形");
            QAction* selectedAction = contextMenu.exec(ui->templateView->mapToGlobal(pos));
            
            if (selectedAction == clearAction) {
                clearUserRect();
            }
        }
    });
}

void MatchToolDialog::setupConnections()
{
    // 连接信号槽
    connect(ui->loadSrcButton, &QPushButton::clicked, this, &MatchToolDialog::onLoadSrcButtonClicked);
    connect(ui->loadDstButton, &QPushButton::clicked, this, &MatchToolDialog::onLoadDstButtonClicked);
    connect(ui->executeButton, &QPushButton::clicked, this, &MatchToolDialog::onExecuteButtonClicked);
    connect(ui->clearButton, &QPushButton::clicked, this, &MatchToolDialog::onClearButtonClicked);
    
    // 连接参数变化信号
    connect(ui->maxPositionsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MatchToolDialog::onParameterChanged);
    connect(ui->maxOverlapSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MatchToolDialog::onParameterChanged);
    connect(ui->scoreSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &MatchToolDialog::onParameterChanged);
}

void MatchToolDialog::onLoadSrcButtonClicked()
{
    // 如果有保存的源图像路径，使用它作为起始目录
    QString startDir = m_lastSourceImagePath.isEmpty() ? m_lastDirectory : QFileInfo(m_lastSourceImagePath).absolutePath();
    
    QString filePath = QFileDialog::getOpenFileName(this, "选择源图像", startDir, "图像文件 (*.png *.jpg *.bmp *.jpeg *.tiff)");
    if (!filePath.isEmpty()) {
        m_lastDirectory = QFileInfo(filePath).absolutePath();
        m_lastSourceImagePath = filePath;
        loadSourceImage(filePath);
        saveImagePaths(); // 保存路径
    }
}

void MatchToolDialog::onLoadDstButtonClicked()
{
    // 如果有保存的模板图像路径，使用它作为起始目录
    QString startDir = m_lastTemplateImagePath.isEmpty() ? m_lastDirectory : QFileInfo(m_lastTemplateImagePath).absolutePath();
    
    QString filePath = QFileDialog::getOpenFileName(this, "选择模板图像", startDir, "图像文件 (*.png *.jpg *.bmp *.jpeg *.tiff)");
    if (!filePath.isEmpty()) {
        m_lastDirectory = QFileInfo(filePath).absolutePath();
        m_lastTemplateImagePath = filePath;
        loadTemplateImage(filePath);
        saveImagePaths(); // 保存路径
    }
}

void MatchToolDialog::onExecuteButtonClicked()
{
    if (!m_sourceImageLoaded || !m_templateImageLoaded) {
        QMessageBox::warning(this, "警告", "请先加载源图像和模板图像");
        return;
    }
    
    // 设置匹配器参数
    double score = ui->scoreSpinBox->value();
    m_matcher.setMaxPositions(ui->maxPositionsSpinBox->value());
    m_matcher.setMaxOverlap(ui->maxOverlapSpinBox->value());
    m_matcher.setScore(score);
    m_matcher.setToleranceAngle(ui->toleranceAngleSpinBox->value());
    m_matcher.setMinReduceArea(ui->minReduceAreaSpinBox->value());
    m_matcher.setUseSIMD(ui->useSIMDCheckBox->isChecked());
    
    std::cout << "UI设置的参数:" << std::endl;
    std::cout << "Score阈值: " << score << std::endl;
    std::cout << "MaxPositions: " << ui->maxPositionsSpinBox->value() << std::endl;
    std::cout << "MaxOverlap: " << ui->maxOverlapSpinBox->value() << std::endl;
    
    // 记录开始时间
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // 执行匹配（不需要重新学习模板，因为已经在loadTemplateImage中学习过了）
    m_matchResults = m_matcher.match(m_sourceImage);
    
    // 记录结束时间
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
    double timeMs = duration.count() / 1000.0; // 转换为毫秒
    
    // 设置显示结果标志（模拟MFC）
    m_bShowResult = true;
    
    // 显示结果
    displayResults();
    
    // 在源图像上显示匹配结果
    refreshSourceViewWithResults();
    
    // 更新执行时间
    updateExecutionTimeLabel(timeMs);
}



void MatchToolDialog::loadSourceImage(const QString& filePath)
{
    if (filePath.isEmpty()) {
        return;
    }
    
    m_sourceImage = cv::imread(filePath.toStdString(), cv::IMREAD_GRAYSCALE);
    if (m_sourceImage.empty()) {
        QMessageBox::warning(this, "错误", "无法加载源图像: " + filePath);
        return;
    }
    
    // 模拟MFC的LoadSrc()处理过程
    // 1. 计算缩放比例
    QRect viewRect = ui->sourceView->rect();
    double dScaleX = viewRect.width() / (double)m_sourceImage.cols;
    double dScaleY = viewRect.height() / (double)m_sourceImage.rows;
    m_dSrcScale = qMin(dScaleX, dScaleY);
    m_dNewScale = m_dSrcScale;
    
    // 2. 设置显示标志
    m_bShowResult = false;
    m_iScaleTimes = 0;
    
    m_sourceImageLoaded = true;
    refreshSourceView();
    
    // 3. 更新状态栏
    updateSourceImageLabel();
}

void MatchToolDialog::loadTemplateImage(const QString& filePath)
{
    m_templateImage = cv::imread(filePath.toStdString(), cv::IMREAD_GRAYSCALE);
    if (m_templateImage.empty()) {
        QMessageBox::warning(this, "错误", "无法加载模板图像: " + filePath);
        return;
    }
    
    // 模拟MFC的LoadDst()处理过程
    // 1. 计算缩放比例
    QRect viewRect = ui->templateView->rect();
    double dScaleX = viewRect.width() / (double)m_templateImage.cols;
    double dScaleY = viewRect.height() / (double)m_templateImage.rows;
    double m_dDstScale = qMin(dScaleX, dScaleY);
    
    // 重置模板视图缩放比例，以便在refreshTemplateView中重新计算
    m_templateScale = 1.0;
    
    m_templateImageLoaded = true;
    m_matcher.learnPattern(m_templateImage);
    refreshTemplateView();
    
    // 2. 更新状态栏
    updateTemplateImageLabel();
}

void MatchToolDialog::displayResults()
{
    // 显示匹配结果
    ui->resultsTable->setRowCount(m_matchResults.size());
    
    for (size_t i = 0; i < m_matchResults.size(); ++i) {
        const auto& result = m_matchResults[i];
        ui->resultsTable->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        ui->resultsTable->setItem(i, 1, new QTableWidgetItem(QString::number(result.dMatchScore, 'f', 3)));
        ui->resultsTable->setItem(i, 2, new QTableWidgetItem(QString::number(result.dMatchedAngle, 'f', 1)));
        ui->resultsTable->setItem(i, 3, new QTableWidgetItem(QString::number(result.ptCenter.x, 'f', 1)));
        ui->resultsTable->setItem(i, 4, new QTableWidgetItem(QString::number(result.ptCenter.y, 'f', 1)));
    }
    
    updateStatusBar(QString("找到 %1 个匹配结果").arg(m_matchResults.size()));
}

void MatchToolDialog::refreshSourceView()
{
    if (m_sourceImage.empty() || !m_sourceScene) return;
    
    m_sourceScene->clear();
    QPixmap pixmap = matToQPixmap(m_sourceImage);
    if (!pixmap.isNull()) {
        m_sourceScene->addPixmap(pixmap);
        ui->sourceView->setSceneRect(pixmap.rect());
        
        // 如果是第一次加载图像，设置初始缩放比例
        if (m_currentScale == 1.0) {
            // 根据图像大小调整显示
            QRectF sceneRect = m_sourceScene->sceneRect();
            QRectF viewRect = ui->sourceView->rect();
            
            // 计算合适的缩放比例
            double scaleX = viewRect.width() / sceneRect.width();
            double scaleY = viewRect.height() / sceneRect.height();
            double scale = qMin(scaleX, scaleY);
            
            // 如果图像太大，缩小显示；如果太小，放大显示
            if (scale < 1.0) {
                ui->sourceView->fitInView(sceneRect, Qt::KeepAspectRatio);
                m_currentScale = scale;
            } else {
                ui->sourceView->setTransform(QTransform::fromScale(scale, scale));
                m_currentScale = scale;
            }
        } else {
            // 保持当前缩放比例
            applyZoom();
        }
    }
}

void MatchToolDialog::refreshTemplateView()
{
    if (m_templateImage.empty() || !m_templateScene) return;
    
    m_templateScene->clear();
    QPixmap pixmap = matToQPixmap(m_templateImage);
    if (!pixmap.isNull()) {
        m_templateScene->addPixmap(pixmap);
        ui->templateView->setSceneRect(pixmap.rect());
        
        // 如果是第一次加载图像，设置初始缩放比例
        if (m_templateScale == 1.0) {
            // 根据图像大小调整显示
            QRectF sceneRect = m_templateScene->sceneRect();
            QRectF viewRect = ui->templateView->rect();
            
            // 计算合适的缩放比例
            double scaleX = viewRect.width() / sceneRect.width();
            double scaleY = viewRect.height() / sceneRect.height();
            double scale = qMin(scaleX, scaleY);
            
            // 如果图像太大，缩小显示；如果太小，放大显示
            if (scale < 1.0) {
                ui->templateView->fitInView(sceneRect, Qt::KeepAspectRatio);
                m_templateScale = scale;
            } else {
                ui->templateView->setTransform(QTransform::fromScale(scale, scale));
                m_templateScale = scale;
            }
        } else {
            // 保持当前缩放比例
            applyTemplateZoom();
        }
        
        // 如果有用户矩形，重新绘制
        if (m_hasUserRect) {
            drawUserRectOnTemplate();
        }
        
        // 如果有用户多边形，重新绘制
        if (m_hasUserPolygon && !m_polygonPoints.empty()) {
            drawPolygonOnTemplate();
        }
    }
}

void MatchToolDialog::updateStatusBar(const QString& message)
{
    ui->statusBar->showMessage(message);
}

void MatchToolDialog::updateSourceImageLabel()
{
    if (m_sourceImage.empty()) {
        m_sourceImageLabel->setText("源图像: 未加载");
    } else {
        QString text = QString("源图像: %1 x %2").arg(m_sourceImage.cols).arg(m_sourceImage.rows);
        m_sourceImageLabel->setText(text);
    }
}

void MatchToolDialog::updateTemplateImageLabel()
{
    if (m_templateImage.empty()) {
        m_templateImageLabel->setText("模板图像: 未加载");
    } else {
        QString text = QString("模板图像: %1 x %2").arg(m_templateImage.cols).arg(m_templateImage.rows);
        m_templateImageLabel->setText(text);
    }
}

void MatchToolDialog::updateExecutionTimeLabel(double timeMs)
{
    QString text = QString("执行时间: %1 ms").arg(timeMs, 0, 'f', 3);
    m_executionTimeLabel->setText(text);
}

void MatchToolDialog::saveSettings()
{
    QSettings settings("FastestImagePatternMatching", "MatchTool");
    
    // 保存基本设置
    settings.setValue("LastDirectory", m_lastDirectory);
    settings.setValue("LastSourceImagePath", m_lastSourceImagePath);
    settings.setValue("LastTemplateImagePath", m_lastTemplateImagePath);
    
    // 保存匹配器参数
    settings.setValue("MaxPositions", ui->maxPositionsSpinBox->value());
    settings.setValue("MaxOverlap", ui->maxOverlapSpinBox->value());
    settings.setValue("Score", ui->scoreSpinBox->value());
    
    // 保存缩放设置
    settings.setValue("CurrentScale", m_currentScale);
    settings.setValue("MinScale", m_minScale);
    settings.setValue("MaxScale", m_maxScale);
    
    // 保存模板视图缩放设置
    settings.setValue("TemplateScale", m_templateScale);
    settings.setValue("TemplateMinScale", m_templateMinScale);
    settings.setValue("TemplateMaxScale", m_templateMaxScale);
    
    std::cout << "设置已保存" << std::endl;
}

void MatchToolDialog::loadSettings()
{
    QSettings settings("FastestImagePatternMatching", "MatchTool");
    
    // 加载基本设置
    m_lastDirectory = settings.value("LastDirectory", QDir::homePath()).toString();
    m_lastSourceImagePath = settings.value("LastSourceImagePath", "").toString();
    m_lastTemplateImagePath = settings.value("LastTemplateImagePath", "").toString();
    
    // 加载匹配器参数
    ui->maxPositionsSpinBox->setValue(settings.value("MaxPositions", 70).toInt());
    ui->maxOverlapSpinBox->setValue(settings.value("MaxOverlap", 0.0).toDouble());
    ui->scoreSpinBox->setValue(settings.value("Score", 0.7).toDouble());
    
    // 加载缩放设置
    m_currentScale = settings.value("CurrentScale", 1.0).toDouble();
    m_minScale = settings.value("MinScale", 0.1).toDouble();
    m_maxScale = settings.value("MaxScale", 10.0).toDouble();
    
    // 加载模板视图缩放设置
    m_templateScale = settings.value("TemplateScale", 1.0).toDouble();
    m_templateMinScale = settings.value("TemplateMinScale", 0.1).toDouble();
    m_templateMaxScale = settings.value("TemplateMaxScale", 10.0).toDouble();
    
    std::cout << "设置已加载" << std::endl;
}

void MatchToolDialog::saveImagePaths()
{
    QSettings settings("FastestImagePatternMatching", "MatchTool");
    settings.setValue("LastSourceImagePath", m_lastSourceImagePath);
    settings.setValue("LastTemplateImagePath", m_lastTemplateImagePath);
}

void MatchToolDialog::loadImagePaths()
{
    QSettings settings("FastestImagePatternMatching", "MatchTool");
    m_lastSourceImagePath = settings.value("LastSourceImagePath", "").toString();
    m_lastTemplateImagePath = settings.value("LastTemplateImagePath", "").toString();
}

void MatchToolDialog::onParameterChanged()
{
    // 参数变化时自动保存设置
    saveSettings();
}

QImage MatchToolDialog::matToQImage(const cv::Mat& mat)
{
    if (mat.empty()) {
        return QImage();
    }
    
    if (mat.type() == CV_8UC1) {
        // 灰度图像
        return QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
    } else if (mat.type() == CV_8UC3) {
        // BGR图像 - 修复：需要深拷贝，因为QImage需要连续的内存
        cv::Mat rgb;
        cv::cvtColor(mat, rgb, cv::COLOR_BGR2RGB);
        return QImage(rgb.data, rgb.cols, rgb.rows, rgb.step, QImage::Format_RGB888).copy();
    } else {
        std::cout << "不支持的图像类型: " << mat.type() << std::endl;
    }
    return QImage();
}

QPixmap MatchToolDialog::matToQPixmap(const cv::Mat& mat)
{
    return QPixmap::fromImage(matToQImage(mat));
}

void MatchToolDialog::refreshSourceViewWithResults()
{
    if (m_sourceImage.empty() || !m_sourceScene) return;
    
    std::cout << "开始绘制匹配结果，结果数量: " << m_matchResults.size() << std::endl;
    
    // 模拟MFC的RefreshSrcView()处理过程
    // 只有在m_bShowResult为true时才显示结果
    if (!m_bShowResult) {
        refreshSourceView();
        return;
    }
    
    // 创建彩色图像用于显示结果
    cv::Mat displayImage;
    if (m_sourceImage.channels() == 1) {
        cv::cvtColor(m_sourceImage, displayImage, cv::COLOR_GRAY2BGR);
    } else {
        displayImage = m_sourceImage.clone();
    }
    
    std::cout << "原始图像尺寸: " << m_sourceImage.size() << ", 类型: " << m_sourceImage.type() << std::endl;
    std::cout << "显示图像尺寸: " << displayImage.size() << ", 类型: " << displayImage.type() << std::endl;
    
    // 在图像上绘制匹配结果 - 模拟MFC的RefreshSrcView()
    for (size_t i = 0; i < m_matchResults.size(); i++) {
        std::cout << "绘制第 " << i << " 个匹配结果" << std::endl;
        const auto& result = m_matchResults[i];
        
        // 绘制优化的矩形框 - 使用更清晰的颜色和线条
        cv::Point2d ptLT(result.ptLT.x, result.ptLT.y);
        cv::Point2d ptRT(result.ptRT.x, result.ptRT.y);
        cv::Point2d ptLB(result.ptLB.x, result.ptLB.y);
        cv::Point2d ptRB(result.ptRB.x, result.ptRB.y);
        cv::Point2d ptC(result.ptCenter.x, result.ptCenter.y);
        
        // 使用更鲜明的绿色绘制主矩形边框
        cv::Scalar mainColor(0, 200, 0);  // 更亮的绿色
        cv::Scalar accentColor(255, 255, 0);  // 黄色作为强调色
        
        // 绘制主矩形边框 - 使用抗锯齿
        cv::line(displayImage, ptLT, ptLB, mainColor, 2, cv::LINE_AA);
        cv::line(displayImage, ptLB, ptRB, mainColor, 2, cv::LINE_AA);
        cv::line(displayImage, ptRB, ptRT, mainColor, 2, cv::LINE_AA);
        cv::line(displayImage, ptRT, ptLT, mainColor, 2, cv::LINE_AA);
        
        // 绘制角落标记 - 使用更明显的样式
        cv::Point2d ptDis1, ptDis2;
        double rectWidth = cv::norm(ptRT - ptLT);
        double rectHeight = cv::norm(ptLB - ptLT);
        
        if (rectWidth > rectHeight) {
            ptDis1 = (ptLB - ptLT) / 4.0;  // 减少角落标记长度，使其更精致
            ptDis2 = (ptRT - ptLT) / 4.0 * (rectHeight / rectWidth);
        } else {
            ptDis1 = (ptLB - ptLT) / 4.0 * (rectWidth / rectHeight);
            ptDis2 = (ptRT - ptLT) / 4.0;
        }
        
        // 绘制角落线条 - 使用黄色强调
        cv::line(displayImage, ptLT, ptLT + ptDis1/2, accentColor, 1, cv::LINE_AA);
        cv::line(displayImage, ptLT, ptLT + ptDis2/2, accentColor, 1, cv::LINE_AA);
        cv::line(displayImage, ptRT, ptRT + ptDis1/2, accentColor, 1, cv::LINE_AA);
        cv::line(displayImage, ptRT, ptRT - ptDis2/2, accentColor, 1, cv::LINE_AA);
        cv::line(displayImage, ptRB, ptRB - ptDis1/2, accentColor, 1, cv::LINE_AA);
        cv::line(displayImage, ptRB, ptRB - ptDis2/2, accentColor, 1, cv::LINE_AA);
        cv::line(displayImage, ptLB, ptLB - ptDis1/2, accentColor, 1, cv::LINE_AA);
        cv::line(displayImage, ptLB, ptLB + ptDis2/2, accentColor, 1, cv::LINE_AA);
        
        // 绘制中心圆点 - 使用红色，更明显
        cv::circle(displayImage, ptC, 6, cv::Scalar(0, 0, 255), -1, cv::LINE_AA);
        // 添加白色边框
        cv::circle(displayImage, ptC, 6, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
        
        // 绘制序号和分数信息 - 使用更好的字体和颜色
        std::string indexText = std::to_string(i);
        std::string scoreText = "S:" + std::to_string(static_cast<int>(result.dMatchScore * 100));
        std::string angleText = "A:" + std::to_string(static_cast<int>(result.dMatchedAngle));
        
        // 计算文本位置
        cv::Point textPos1(ptLT.x + 15, ptLT.y + 25);
        cv::Point textPos2(ptLT.x + 15, ptLT.y + 45);
        cv::Point textPos3(ptLT.x + 15, ptLT.y + 65);
        
        // 绘制文本背景矩形，提高可读性
        cv::Rect textBgRect(ptLT.x + 10, ptLT.y + 10, 80, 70);
        cv::rectangle(displayImage, textBgRect, cv::Scalar(0, 0, 0), -1);
        cv::rectangle(displayImage, textBgRect, cv::Scalar(255, 255, 255), 1);
        
        // 绘制文本
        cv::putText(displayImage, indexText, textPos1, 
                   cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 0), 2);
        cv::putText(displayImage, scoreText, textPos2, 
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0, 255, 255), 2);
        cv::putText(displayImage, angleText, textPos3, 
                   cv::FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(255, 0, 255), 2);
        
        // 绘制用户定义的矩形（如果存在）
        if (m_hasUserRect) {
            if (m_hasUserPolygon && !m_polygonPoints.empty()) {
                // 绘制用户定义的多边形
                drawUserPolygonOnResults(result, displayImage);
            } else {
                // 绘制用户定义的矩形
                cv::Rect transformedRect = transformRectToResult(m_userDefinedRect, result);
                
                // 使用优化的虚线绘制用户矩形
                cv::Point2d rectLT(transformedRect.x, transformedRect.y);
                cv::Point2d rectRT(transformedRect.x + transformedRect.width, transformedRect.y);
                cv::Point2d rectRB(transformedRect.x + transformedRect.width, transformedRect.y + transformedRect.height);
                cv::Point2d rectLB(transformedRect.x, transformedRect.y + transformedRect.height);
                
                // 使用优化的虚线绘制
                cv::Scalar userRectColor(0, 100, 255);  // 蓝色
                drawDashLine(displayImage, rectLT, rectRT, userRectColor, userRectColor);
                drawDashLine(displayImage, rectRT, rectRB, userRectColor, userRectColor);
                drawDashLine(displayImage, rectRB, rectLB, userRectColor, userRectColor);
                drawDashLine(displayImage, rectLB, rectLT, userRectColor, userRectColor);
                
                // 在矩形中心绘制小圆点
                cv::Point2d rectCenter(transformedRect.x + transformedRect.width/2, transformedRect.y + transformedRect.height/2);
                cv::circle(displayImage, rectCenter, 5, userRectColor, -1, cv::LINE_AA);
                cv::circle(displayImage, rectCenter, 5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
                
                // 在矩形四个角绘制小方块，增强可见性
                int cornerSize = 4;
                cv::Scalar cornerColor(255, 255, 255);  // 白色角落标记
                cv::rectangle(displayImage, 
                             cv::Rect(rectLT.x - cornerSize/2, rectLT.y - cornerSize/2, cornerSize, cornerSize),
                             cornerColor, -1);
                cv::rectangle(displayImage, 
                             cv::Rect(rectRT.x - cornerSize/2, rectRT.y - cornerSize/2, cornerSize, cornerSize),
                             cornerColor, -1);
                cv::rectangle(displayImage, 
                             cv::Rect(rectRB.x - cornerSize/2, rectRB.y - cornerSize/2, cornerSize, cornerSize),
                             cornerColor, -1);
                cv::rectangle(displayImage, 
                             cv::Rect(rectLB.x - cornerSize/2, rectLB.y - cornerSize/2, cornerSize, cornerSize),
                             cornerColor, -1);
            }
        }
        
    }
    // cv::imwrite("result_src.png", displayImage);    
    std::cout << "保存的图片尺寸: " << displayImage.size() << ", 类型: " << displayImage.type() << std::endl;
    
    // 更新显示
    m_sourceScene->clear();
    QPixmap pixmap = matToQPixmap(displayImage);
    std::cout << "QPixmap是否为空: " << pixmap.isNull() << ", 尺寸: " << pixmap.size().width() << "x" << pixmap.size().height() << std::endl;
    
    if (!pixmap.isNull()) {
        m_sourceScene->addPixmap(pixmap);
        ui->sourceView->setSceneRect(pixmap.rect());
        
        // 保持当前缩放比例
        applyZoom();
    } else {
        std::cout << "QPixmap转换失败！" << std::endl;
    }
}

void MatchToolDialog::dragEnterEvent(QDragEnterEvent *event)
{
    // 拖拽进入事件
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MatchToolDialog::dropEvent(QDropEvent *event)
{
    // 拖拽释放事件
    const QMimeData *mimeData = event->mimeData();
    if (mimeData->hasUrls()) {
        QList<QUrl> urlList = mimeData->urls();
        if (!urlList.isEmpty()) {
            QString filePath = urlList.first().toLocalFile();
            if (!filePath.isEmpty()) {
                loadSourceImage(filePath);
            }
        }
    }
}

void MatchToolDialog::drawDashLine(cv::Mat& matDraw, cv::Point2d ptStart, cv::Point2d ptEnd, cv::Scalar color1, cv::Scalar color2)
{
    // 绘制优化的虚线
    double dx = ptEnd.x - ptStart.x;
    double dy = ptEnd.y - ptStart.y;
    double length = sqrt(dx*dx + dy*dy);
    
    if (length < 1) return;
    
    // 根据线条长度动态调整虚线参数
    int dashLength = std::max(6, static_cast<int>(length * 0.1));
    int gapLength = std::max(3, static_cast<int>(length * 0.05));
    double step = dashLength + gapLength;
    
    cv::Point2d direction(dx/length, dy/length);
    cv::Point2d current = ptStart;
    
    // 使用抗锯齿线条绘制
    int lineThickness = 1;
    
    while (cv::norm(current - ptStart) < length) {
        cv::Point2d end = current + direction * dashLength;
        if (cv::norm(end - ptStart) > length) {
            end = ptEnd;
        }
        
        // 绘制虚线段，使用更粗的线条确保可见性
        cv::line(matDraw, current, end, color1, lineThickness, cv::LINE_AA);
        
        current = end + direction * gapLength;
    }
}

void MatchToolDialog::drawMarkCross(cv::Mat& matDraw, double x, double y, int length, cv::Scalar color, int thickness)
{
    cv::Point2d center(x, y);
    cv::Point2d pt1 = center - cv::Point2d(length/2, 0);
    cv::Point2d pt2 = center + cv::Point2d(length/2, 0);
    cv::Point2d pt3 = center - cv::Point2d(0, length/2);
    cv::Point2d pt4 = center + cv::Point2d(0, length/2);
    
    cv::line(matDraw, pt1, pt2, color, thickness);
    cv::line(matDraw, pt3, pt4, color, thickness);
}

void MatchToolDialog::onClearButtonClicked()
{
    // 清除结果
    m_matchResults.clear();
    ui->resultsTable->setRowCount(0);
    refreshSourceView();
    updateStatusBar("结果已清除");
}

bool MatchToolDialog::eventFilter(QObject *obj, QEvent *event)
{
    // 处理源视图的鼠标滚轮缩放和矩形绘制
    if (obj == ui->sourceView->viewport()) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
            
            // 检查是否按住Ctrl键
            if (wheelEvent->modifiers() & Qt::ControlModifier) {
                // 计算缩放因子
                double zoomFactor = 1.15;
                if (wheelEvent->angleDelta().y() > 0) {
                    // 放大
                    m_currentScale *= zoomFactor;
                } else {
                    // 缩小
                    m_currentScale /= zoomFactor;
                }
                
                // 限制缩放范围
                m_currentScale = qBound(m_minScale, m_currentScale, m_maxScale);
                
                // 应用以鼠标为中心的缩放
                applyZoomAtMousePosition(wheelEvent->pos());
                
                // 阻止事件继续传播
                return true;
            }
        }
    }
    
    // 处理模板视图的鼠标事件
    if (obj == ui->templateView->viewport()) {
        if (event->type() == QEvent::Wheel) {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent*>(event);
            
            // 检查是否按住Ctrl键
            if (wheelEvent->modifiers() & Qt::ControlModifier) {
                // 计算缩放因子
                double zoomFactor = 1.15;
                if (wheelEvent->angleDelta().y() > 0) {
                    // 放大
                    m_templateScale *= zoomFactor;
                } else {
                    // 缩小
                    m_templateScale /= zoomFactor;
                }
                
                // 限制缩放范围
                m_templateScale = qBound(m_templateMinScale, m_templateScale, m_templateMaxScale);
                
                // 应用以鼠标为中心的缩放
                applyTemplateZoomAtMousePosition(wheelEvent->pos());
                
                // 阻止事件继续传播
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonPress) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton && m_templateImageLoaded) {
                onTemplateViewMousePress(mouseEvent);
                return true;
            } else if (mouseEvent->button() == Qt::RightButton && m_templateImageLoaded) {
                onTemplateViewRightClick(mouseEvent);
                return true;
            }
        } else if (event->type() == QEvent::MouseMove) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (m_isDrawingRect || m_isSelectingPolygon) {
                onTemplateViewMouseMove(mouseEvent);
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton && (m_isDrawingRect || m_isSelectingPolygon)) {
                onTemplateViewMouseRelease(mouseEvent);
                return true;
            }
        }
    }
    
    // 调用基类的事件过滤器
    return QDialog::eventFilter(obj, event);
}

void MatchToolDialog::applyZoom()
{
    if (!m_sourceScene || m_sourceImage.empty()) {
        return;
    }
    
    // 应用缩放变换
    QTransform transform;
    transform.scale(m_currentScale, m_currentScale);
    ui->sourceView->setTransform(transform);
    
    // 更新状态栏显示当前缩放比例（临时显示）
    ui->statusBar->showMessage(QString("缩放比例: %1%").arg(m_currentScale * 100, 0, 'f', 1), 2000);
}

void MatchToolDialog::applyTemplateZoom()
{
    if (!m_templateScene || m_templateImage.empty()) {
        return;
    }
    
    // 应用缩放变换
    QTransform transform;
    transform.scale(m_templateScale, m_templateScale);
    ui->templateView->setTransform(transform);
    
    // 更新状态栏显示当前缩放比例（临时显示）
    ui->statusBar->showMessage(QString("模板缩放比例: %1%").arg(m_templateScale * 100, 0, 'f', 1), 2000);
}

// 模板视图鼠标事件处理
void MatchToolDialog::onTemplateViewMousePress(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_templateImageLoaded) {
        if (m_isSelectingPolygon) {
            // 多边形选择模式：添加点
            addPolygonPoint(event->pos());
        } else {
            // 矩形绘制模式（保持向后兼容）
            m_isDrawingRect = true;
            m_rectStartPoint = ui->templateView->mapToScene(event->pos()).toPoint();
            m_rectEndPoint = m_rectStartPoint;
            
            // 清除之前的矩形
            m_hasUserRect = false;
            refreshTemplateView();
        }
    }
}

void MatchToolDialog::onTemplateViewMouseMove(QMouseEvent* event)
{
    if (m_isDrawingRect && m_templateImageLoaded) {
        m_rectEndPoint = ui->templateView->mapToScene(event->pos()).toPoint();
        
        // 实时显示绘制的矩形
        refreshTemplateView();
        drawUserRectOnTemplate();
        
        // 添加实时尺寸显示
        if (m_templateScene) {
            // 清除之前的实时尺寸标签
            QList<QGraphicsItem*> items = m_templateScene->items();
            for (auto item : items) {
                if (item->data(0).toString() == "LiveSizeLabel") {
                    m_templateScene->removeItem(item);
                    delete item;
                }
            }
            
            // 计算实时矩形尺寸
            int width = abs(m_rectEndPoint.x() - m_rectStartPoint.x());
            int height = abs(m_rectEndPoint.y() - m_rectStartPoint.y());
            
            // 创建实时尺寸标签
            QGraphicsTextItem* sizeLabel = new QGraphicsTextItem();
            sizeLabel->setPlainText(QString("%1 x %2").arg(width).arg(height));
            sizeLabel->setData(0, "LiveSizeLabel");
            
            // 设置标签样式
            QFont font = sizeLabel->font();
            font.setPointSize(10);
            font.setBold(true);
            sizeLabel->setFont(font);
            sizeLabel->setDefaultTextColor(QColor(255, 255, 255));
            
            // 设置标签位置（在矩形下方）
            sizeLabel->setPos((m_rectStartPoint.x() + m_rectEndPoint.x()) / 2 - sizeLabel->boundingRect().width() / 2, 
                              (m_rectStartPoint.y() + m_rectEndPoint.y()) / 2 + sizeLabel->boundingRect().height() + 10);
            
            // 添加黑色背景以提高可读性
            QGraphicsRectItem* bgRect = new QGraphicsRectItem(sizeLabel->boundingRect());
            bgRect->setBrush(QColor(0, 0, 0, 180));
            bgRect->setPen(Qt::NoPen);
            bgRect->setPos(sizeLabel->pos());
            bgRect->setData(0, "LiveSizeLabel");
            
            // 添加到场景
            m_templateScene->addItem(bgRect);
            m_templateScene->addItem(sizeLabel);
        }
    } else if (m_isSelectingPolygon && m_templateImageLoaded) {
        // 多边形选择模式：实时显示当前鼠标位置和已选择的点
        refreshTemplateView();
        drawPolygonOnTemplate();
        
        // 显示实时预览线（从最后一个点到当前鼠标位置）
        if (!m_polygonPoints.empty()) {
            QPointF currentPos = ui->templateView->mapToScene(event->pos());
            QPointF lastPoint(m_polygonPoints.back().x, m_polygonPoints.back().y);
            
            // 创建预览线
            QGraphicsLineItem* previewLine = new QGraphicsLineItem(lastPoint.x(), lastPoint.y(), 
                                                                  currentPos.x(), currentPos.y());
            previewLine->setPen(QPen(QColor(255, 255, 0), 2, Qt::DashLine));  // 黄色虚线
            previewLine->setData(0, "PreviewLine");
            m_templateScene->addItem(previewLine);
        }
    }
}

void MatchToolDialog::onTemplateViewMouseRelease(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && m_isDrawingRect) {
        m_isDrawingRect = false;
        
        // 计算最终矩形
        int x = qMin(m_rectStartPoint.x(), m_rectEndPoint.x());
        int y = qMin(m_rectStartPoint.y(), m_rectEndPoint.y());
        int width = abs(m_rectEndPoint.x() - m_rectStartPoint.x());
        int height = abs(m_rectEndPoint.y() - m_rectStartPoint.y());
        
        if (width > 5 && height > 5) {
            m_userDefinedRect = cv::Rect(x, y, width, height);
            m_hasUserRect = true;
            
            // 清除实时尺寸标签
            if (m_templateScene) {
                QList<QGraphicsItem*> items = m_templateScene->items();
                for (auto item : items) {
                    if (item->data(0).toString() == "LiveSizeLabel") {
                        m_templateScene->removeItem(item);
                        delete item;
                    }
                }
            }
            
            // 绘制最终矩形
            refreshTemplateView();
            drawUserRectOnTemplate();
            
            // 更新状态栏
            ui->statusBar->showMessage(QString("用户矩形已设置: %1 x %2").arg(width).arg(height), 3000);
        } else {
            // 矩形太小，清除
            m_hasUserRect = false;
            refreshTemplateView();
            ui->statusBar->showMessage("矩形太小，已清除", 2000);
        }
    }
}

void MatchToolDialog::onTemplateViewRightClick(QMouseEvent* event)
{
    if (event->button() == Qt::RightButton && m_templateImageLoaded) {
        if (m_isSelectingPolygon) {
            // 右键结束多边形选择
            finishPolygonSelection();
        } else {
            // 开始多边形选择模式
            startPolygonSelection();
        }
    }
}

void MatchToolDialog::startPolygonSelection()
{
    // 清除之前的矩形选择
    if (m_hasUserRect) {
        clearUserRect();
    }
    
    // 开始多边形选择模式
    m_isSelectingPolygon = true;
    m_polygonPoints.clear();
    m_hasUserPolygon = false;
    
    // 更新状态栏
    ui->statusBar->showMessage("多边形选择模式：左键添加点，右键结束选择", 5000);
    
    // 刷新模板视图
    refreshTemplateView();
}

void MatchToolDialog::finishPolygonSelection()
{
    if (m_polygonPoints.size() < 3) {
        ui->statusBar->showMessage("多边形至少需要3个点", 3000);
        m_isSelectingPolygon = false;
        m_polygonPoints.clear();
        refreshTemplateView();
        return;
    }

    // 直接保存多边形，不创建矩形边界框
    m_hasUserPolygon = true;
    m_hasUserRect = true;  // 保持兼容性
    
    // 清除实时预览线
    if (m_templateScene) {
        QList<QGraphicsItem*> items = m_templateScene->items();
        for (auto item : items) {
            QString itemType = item->data(0).toString();
            if (itemType == "PreviewLine") {
                m_templateScene->removeItem(item);
                delete item;
            }
        }
    }

    // 绘制最终多边形
    refreshTemplateView();
    drawPolygonOnTemplate();

    // 更新状态栏
    ui->statusBar->showMessage(QString("用户多边形已设置: %1 个点").arg(m_polygonPoints.size()), 3000);
    
    // 结束多边形选择模式
    m_isSelectingPolygon = false;
}

void MatchToolDialog::addPolygonPoint(const QPoint& pos)
{
    if (!m_isSelectingPolygon) return;
    
    // 将屏幕坐标转换为场景坐标
    QPointF scenePos = ui->templateView->mapToScene(pos);
    
    // 添加点到多边形
    m_polygonPoints.push_back(cv::Point2f(scenePos.x(), scenePos.y()));
    
    // 更新显示
    refreshTemplateView();
    drawPolygonOnTemplate();
    
    // 更新状态栏
    ui->statusBar->showMessage(QString("已添加点 %1，当前共 %2 个点").arg(m_polygonPoints.size()).arg(m_polygonPoints.size()), 2000);
}

void MatchToolDialog::drawPolygonOnTemplate()
{
    if (m_polygonPoints.empty() || !m_templateScene) return;
    
    // 清除之前的预览线
    QList<QGraphicsItem*> items = m_templateScene->items();
    for (auto item : items) {
        if (item->data(0).toString() == "PreviewLine") {
            m_templateScene->removeItem(item);
            delete item;
        }
    }
    
    // 绘制已选择的点
    for (size_t i = 0; i < m_polygonPoints.size(); i++) {
        const auto& point = m_polygonPoints[i];
        
        // 绘制点
        QGraphicsEllipseItem* pointItem = new QGraphicsEllipseItem(point.x - 3, point.y - 3, 6, 6);
        pointItem->setBrush(QColor(255, 0, 0));  // 红色填充
        pointItem->setPen(QPen(QColor(255, 255, 255), 1));  // 白色边框
        pointItem->setData(0, "PolygonPoint");
        m_templateScene->addItem(pointItem);
        
        // 绘制点的序号
        QGraphicsTextItem* indexLabel = new QGraphicsTextItem(QString::number(i + 1));
        indexLabel->setPos(point.x + 5, point.y - 10);
        indexLabel->setDefaultTextColor(QColor(255, 255, 255));
        QFont font = indexLabel->font();
        font.setPointSize(8);
        font.setBold(true);
        indexLabel->setFont(font);
        indexLabel->setData(0, "PolygonPoint");
        m_templateScene->addItem(indexLabel);
        
        // 绘制连接线（除了最后一个点）
        if (i < m_polygonPoints.size() - 1) {
            const auto& nextPoint = m_polygonPoints[i + 1];
            QGraphicsLineItem* lineItem = new QGraphicsLineItem(point.x, point.y, nextPoint.x, nextPoint.y);
            lineItem->setPen(QPen(QColor(0, 255, 0), 2));  // 绿色线条
            lineItem->setData(0, "PolygonPoint");
            m_templateScene->addItem(lineItem);
        }
    }
    
    // 如果有多于2个点，绘制闭合线
    if (m_polygonPoints.size() > 2) {
        const auto& firstPoint = m_polygonPoints.front();
        const auto& lastPoint = m_polygonPoints.back();
        QGraphicsLineItem* closeLine = new QGraphicsLineItem(lastPoint.x, lastPoint.y, firstPoint.x, firstPoint.y);
        closeLine->setPen(QPen(QColor(0, 255, 0), 2, Qt::DashLine));  // 绿色虚线
        closeLine->setData(0, "PolygonPoint");
        m_templateScene->addItem(closeLine);
    }
}

void MatchToolDialog::drawUserRectOnTemplate()
{
    if (!m_hasUserRect || !m_templateScene) return;
    
    // 清除之前的矩形图形项
    QList<QGraphicsItem*> items = m_templateScene->items();
    for (auto item : items) {
        if (item->data(0).toString() == "UserRect") {
            m_templateScene->removeItem(item);
            delete item;
        }
    }
    
    // 创建新的矩形图形项
    QGraphicsRectItem* rectItem = new QGraphicsRectItem(
        m_userDefinedRect.x, m_userDefinedRect.y, 
        m_userDefinedRect.width, m_userDefinedRect.height);
    
    // 设置优化的矩形样式
    QPen pen(QColor(0, 100, 255), 3, Qt::DashLine);  // 蓝色虚线，更粗
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    rectItem->setPen(pen);
    rectItem->setBrush(Qt::NoBrush);
    
    // 设置标识数据
    rectItem->setData(0, "UserRect");
    
    // 添加到场景
    m_templateScene->addItem(rectItem);
    
    // 添加角落标记
    addCornerMarkers(m_userDefinedRect);
    
    // 添加尺寸标签
    addSizeLabel(m_userDefinedRect);
}

void MatchToolDialog::addCornerMarkers(const cv::Rect& rect)
{
    if (!m_templateScene) return;
    
    // 清除之前的角落标记
    QList<QGraphicsItem*> items = m_templateScene->items();
    for (auto item : items) {
        if (item->data(0).toString() == "CornerMarker") {
            m_templateScene->removeItem(item);
            delete item;
        }
    }
    
    // 创建四个角落的标记
    int cornerSize = 6;
    QColor cornerColor(255, 255, 255);  // 白色角落标记
    
    // 左上角
    QGraphicsRectItem* cornerLT = new QGraphicsRectItem(
        rect.x - cornerSize/2, rect.y - cornerSize/2, cornerSize, cornerSize);
    cornerLT->setBrush(cornerColor);
    cornerLT->setPen(Qt::NoPen);
    cornerLT->setData(0, "CornerMarker");
    m_templateScene->addItem(cornerLT);
    
    // 右上角
    QGraphicsRectItem* cornerRT = new QGraphicsRectItem(
        rect.x + rect.width - cornerSize/2, rect.y - cornerSize/2, cornerSize, cornerSize);
    cornerRT->setBrush(cornerColor);
    cornerRT->setPen(Qt::NoPen);
    cornerRT->setData(0, "CornerMarker");
    m_templateScene->addItem(cornerRT);
    
    // 右下角
    QGraphicsRectItem* cornerRB = new QGraphicsRectItem(
        rect.x + rect.width - cornerSize/2, rect.y + rect.height - cornerSize/2, cornerSize, cornerSize);
    cornerRB->setBrush(cornerColor);
    cornerRB->setPen(Qt::NoPen);
    cornerRB->setData(0, "CornerMarker");
    m_templateScene->addItem(cornerRB);
    
    // 左下角
    QGraphicsRectItem* cornerLB = new QGraphicsRectItem(
        rect.x - cornerSize/2, rect.y + rect.height - cornerSize/2, cornerSize, cornerSize);
    cornerLB->setBrush(cornerColor);
    cornerLB->setPen(Qt::NoPen);
    cornerLB->setData(0, "CornerMarker");
    m_templateScene->addItem(cornerLB);
}

void MatchToolDialog::addSizeLabel(const cv::Rect& rect)
{
    if (!m_templateScene) return;
    
    // 清除之前的尺寸标签
    QList<QGraphicsItem*> items = m_templateScene->items();
    for (auto item : items) {
        if (item->data(0).toString() == "SizeLabel") {
            m_templateScene->removeItem(item);
            delete item;
        }
    }
    
    // 创建尺寸标签
    QString sizeText = QString("%1 × %2").arg(rect.width).arg(rect.height);
    QGraphicsTextItem* sizeLabel = new QGraphicsTextItem(sizeText);
    
    // 设置标签样式
    QFont font = sizeLabel->font();
    font.setPointSize(10);
    font.setBold(true);
    sizeLabel->setFont(font);
    sizeLabel->setDefaultTextColor(QColor(255, 255, 255));
    
    // 设置标签位置（在矩形下方）
    sizeLabel->setPos(rect.x + rect.width/2 - sizeLabel->boundingRect().width()/2, 
                      rect.y + rect.height + 10);
    
    // 添加黑色背景以提高可读性
    QGraphicsRectItem* bgRect = new QGraphicsRectItem(sizeLabel->boundingRect());
    bgRect->setBrush(QColor(0, 0, 0, 180));
    bgRect->setPen(Qt::NoPen);
    bgRect->setPos(sizeLabel->pos());
    bgRect->setData(0, "SizeLabel");
    
    // 添加到场景
    m_templateScene->addItem(bgRect);
    sizeLabel->setData(0, "SizeLabel");
    m_templateScene->addItem(sizeLabel);
}

void MatchToolDialog::drawUserRectOnResults()
{
    if (!m_hasUserRect || m_matchResults.empty()) return;
    
    // 在匹配结果上绘制用户定义的矩形
    for (const auto& result : m_matchResults) {
        // 计算旋转后的矩形角点
        cv::Point2f templateRectCenter(
            m_userDefinedRect.x + m_userDefinedRect.width / 2.0f,
            m_userDefinedRect.y + m_userDefinedRect.height / 2.0f
        );
        
        cv::Point2f templateImageCenter(
            m_templateImage.cols / 2.0f,
            m_templateImage.rows / 2.0f
        );
        
        cv::Point2f rectOffset = templateRectCenter - templateImageCenter;
        
        // 计算模板矩形的四个角点（相对于模板矩形中心）
        cv::Point2f rectCorners[4] = {
            cv::Point2f(-m_userDefinedRect.width / 2.0f, -m_userDefinedRect.height / 2.0f),  // 左上
            cv::Point2f(m_userDefinedRect.width / 2.0f, -m_userDefinedRect.height / 2.0f),   // 右上
            cv::Point2f(m_userDefinedRect.width / 2.0f, m_userDefinedRect.height / 2.0f),    // 右下
            cv::Point2f(-m_userDefinedRect.width / 2.0f, m_userDefinedRect.height / 2.0f)    // 左下
        };
        
        // 应用旋转变换
        double angle = result.dMatchedAngle * CV_PI / 180.0;
        double cosA = cos(angle);
        double sinA = sin(angle);
        
        cv::Point2f transformedCorners[4];
        for (int i = 0; i < 4; i++) {
            // 1. 先旋转矩形角点
            cv::Point2f rotatedPoint;
            rotatedPoint.x = rectCorners[i].x * cosA - rectCorners[i].y * sinA;
            rotatedPoint.y = rectCorners[i].x * sinA + rectCorners[i].y * cosA;
            
            // 2. 加上矩形偏移（考虑旋转后的偏移）
            cv::Point2f rotatedOffset;
            rotatedOffset.x = rectOffset.x * cosA - rectOffset.y * sinA;
            rotatedOffset.y = rectOffset.x * sinA + rectOffset.y * cosA;
            
            // 3. 移动到匹配位置
            transformedCorners[i] = rotatedPoint + rotatedOffset + cv::Point2f(result.ptCenter.x, result.ptCenter.y);
        }
        
        // 在结果图像上绘制旋转后的矩形
        // 这里需要在refreshSourceViewWithResults中调用
    }
}

cv::Rect MatchToolDialog::transformRectToResult(const cv::Rect& templateRect, const s_SingleTargetMatch& match)
{
    // 计算模板矩形的中心点（相对于模板图像）
    cv::Point2f templateRectCenter(
        templateRect.x + templateRect.width / 2.0f,
        templateRect.y + templateRect.height / 2.0f
    );
    
    // 计算模板图像的中心点
    cv::Point2f templateImageCenter(
        m_templateImage.cols / 2.0f,
        m_templateImage.rows / 2.0f
    );
    
    // 计算模板矩形相对于模板图像中心的偏移
    cv::Point2f rectOffset = templateRectCenter - templateImageCenter;
    
    // 计算模板矩形的四个角点（相对于模板矩形中心）
    cv::Point2f rectCorners[4] = {
        cv::Point2f(-templateRect.width / 2.0f, -templateRect.height / 2.0f),  // 左上
        cv::Point2f(templateRect.width / 2.0f, -templateRect.height / 2.0f),   // 右上
        cv::Point2f(templateRect.width / 2.0f, templateRect.height / 2.0f),    // 右下
        cv::Point2f(-templateRect.width / 2.0f, templateRect.height / 2.0f)    // 左下
    };
    
    // 应用旋转变换
    double angle = match.dMatchedAngle * CV_PI / 180.0;  // 转换为弧度
    double cosA = cos(angle);
    double sinA = sin(angle);
    
    cv::Point2f transformedCorners[4];
    for (int i = 0; i < 4; i++) {
        // 1. 先旋转矩形角点
        cv::Point2f rotatedPoint;
        rotatedPoint.x = rectCorners[i].x * cosA - rectCorners[i].y * sinA;
        rotatedPoint.y = rectCorners[i].x * sinA + rectCorners[i].y * cosA;
        
        // 2. 加上矩形偏移（考虑旋转后的偏移）
        cv::Point2f rotatedOffset;
        rotatedOffset.x = rectOffset.x * cosA - rectOffset.y * sinA;
        rotatedOffset.y = rectOffset.x * sinA + rectOffset.y * cosA;
        
        // 3. 移动到匹配位置
        transformedCorners[i] = rotatedPoint + rotatedOffset + cv::Point2f(match.ptCenter.x, match.ptCenter.y);
    }
    
    // 计算变换后矩形的边界框
    double minX = std::min({transformedCorners[0].x, transformedCorners[1].x, transformedCorners[2].x, transformedCorners[3].x});
    double minY = std::min({transformedCorners[0].y, transformedCorners[1].y, transformedCorners[2].y, transformedCorners[3].y});
    double maxX = std::max({transformedCorners[0].x, transformedCorners[1].x, transformedCorners[2].x, transformedCorners[3].x});
    double maxY = std::max({transformedCorners[0].y, transformedCorners[1].y, transformedCorners[2].y, transformedCorners[3].y});
    
    // 确保坐标在有效范围内
    minX = std::max(0.0, minX);
    minY = std::max(0.0, minY);
    maxX = std::min((double)m_sourceImage.cols, maxX);
    maxY = std::min((double)m_sourceImage.rows, maxY);
    
    return cv::Rect(minX, minY, maxX - minX, maxY - minY);
}

void MatchToolDialog::clearUserRect()
{
    if (!m_hasUserRect && !m_hasUserPolygon) return;
    
    // 清除用户矩形标志
    m_hasUserRect = false;
    m_userDefinedRect = cv::Rect();
    
    // 清除用户多边形标志
    m_hasUserPolygon = false;
    m_polygonPoints.clear();
    m_isSelectingPolygon = false;
    
    // 从匹配器中清除用户矩形
    m_matcher.setUserDefinedRect(cv::Rect());
    
    // 清除模板视图上的所有相关图形项
    if (m_templateScene) {
        QList<QGraphicsItem*> items = m_templateScene->items();
        for (auto item : items) {
            QString itemType = item->data(0).toString();
            if (itemType == "UserRect" || itemType == "CornerMarker" || 
                itemType == "SizeLabel" || itemType == "LiveSizeLabel" ||
                itemType == "PolygonPoint" || itemType == "PreviewLine") {
                m_templateScene->removeItem(item);
                delete item;
            }
        }
    }
    
    // 刷新模板视图
    refreshTemplateView();
    
    // 如果有匹配结果，重新绘制源图像
    if (!m_matchResults.empty()) {
        refreshSourceViewWithResults();
    }
    
    // 显示状态信息
    ui->statusBar->showMessage("用户选择区域已清除", 2000);
}

void MatchToolDialog::applyZoomAtMousePosition(const QPoint& mousePos)
{
    if (!m_sourceScene || m_sourceImage.empty()) {
        return;
    }
    
    // 将鼠标位置转换为场景坐标
    QPointF scenePos = ui->sourceView->mapToScene(mousePos);
    
    // 应用缩放变换
    QTransform transform;
    transform.scale(m_currentScale, m_currentScale);
    ui->sourceView->setTransform(transform);
    
    // 计算新的场景位置，使鼠标位置保持不变
    QPointF newScenePos = ui->sourceView->mapToScene(mousePos);
    QPointF delta = newScenePos - scenePos;
    
    // 调整视图中心，使鼠标位置保持不变
    ui->sourceView->centerOn(scenePos);
    
    // 如果有匹配结果和用户矩形，需要重新绘制以确保矩形正确显示
    if (m_bShowResult && m_hasUserRect && !m_matchResults.empty()) {
        // 延迟重新绘制，避免缩放过程中的频繁重绘
        QTimer::singleShot(50, this, [this]() {
            refreshSourceViewWithResults();
        });
    }
    
    // 更新状态栏显示当前缩放比例（临时显示）
    ui->statusBar->showMessage(QString("缩放比例: %1%").arg(m_currentScale * 100, 0, 'f', 1), 2000);
}

void MatchToolDialog::applyTemplateZoomAtMousePosition(const QPoint& mousePos)
{
    if (!m_templateScene || m_templateImage.empty()) {
        return;
    }
    
    // 将鼠标位置转换为场景坐标
    QPointF scenePos = ui->templateView->mapToScene(mousePos);
    
    // 应用缩放变换
    QTransform transform;
    transform.scale(m_templateScale, m_templateScale);
    ui->templateView->setTransform(transform);
    
    // 计算新的场景位置，使鼠标位置保持不变
    QPointF newScenePos = ui->templateView->mapToScene(mousePos);
    QPointF delta = newScenePos - scenePos;
    
    // 调整视图中心，使鼠标位置保持不变
    ui->templateView->centerOn(scenePos);
    
    // 如果有用户矩形，需要重新绘制以确保矩形正确显示
    if (m_hasUserRect) {
        // 延迟重新绘制，避免缩放过程中的频繁重绘
        QTimer::singleShot(50, this, [this]() {
            refreshTemplateView();
        });
    }
    
    // 更新状态栏显示当前缩放比例（临时显示）
    ui->statusBar->showMessage(QString("模板缩放比例: %1%").arg(m_templateScale * 100, 0, 'f', 1), 2000);
}

void MatchToolDialog::clearPolygon()
{
    if (!m_hasUserPolygon) return;
    
    // 清除多边形标志
    m_hasUserPolygon = false;
    m_polygonPoints.clear();
    m_isSelectingPolygon = false;
    
    // 清除模板视图上的所有相关图形项
    if (m_templateScene) {
        QList<QGraphicsItem*> items = m_templateScene->items();
        for (auto item : items) {
            QString itemType = item->data(0).toString();
            if (itemType == "PolygonPoint" || itemType == "PreviewLine") {
                m_templateScene->removeItem(item);
                delete item;
            }
        }
    }
    
    // 刷新模板视图
    refreshTemplateView();
    
    // 显示状态信息
    ui->statusBar->showMessage("用户多边形已清除", 2000);
}

void MatchToolDialog::drawUserPolygonOnResults(const s_SingleTargetMatch& result, cv::Mat& displayImage)
{
    if (m_polygonPoints.empty()) return;
    
    // 变换多边形到结果图像
    std::vector<cv::Point2f> transformedPolygon = transformPolygonToResult(m_polygonPoints, result);
    
    // 绘制变换后的多边形
    cv::Scalar polygonColor(0, 100, 255);  // 蓝色
    cv::Scalar pointColor(255, 0, 0);      // 红色
    cv::Scalar lineColor(0, 255, 0);       // 绿色
    
    // 绘制多边形边线
    for (size_t i = 0; i < transformedPolygon.size(); i++) {
        const auto& currentPoint = transformedPolygon[i];
        const auto& nextPoint = transformedPolygon[(i + 1) % transformedPolygon.size()];
        
        // 绘制边线
        cv::line(displayImage, currentPoint, nextPoint, polygonColor, 2, cv::LINE_AA);
        
        // 绘制顶点
        cv::circle(displayImage, currentPoint, 4, pointColor, -1, cv::LINE_AA);
        cv::circle(displayImage, currentPoint, 4, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
    }
    
    // 绘制多边形中心点
    cv::Point2f center(0, 0);
    for (const auto& point : transformedPolygon) {
        center += point;
    }
    center.x /= transformedPolygon.size();
    center.y /= transformedPolygon.size();
    
    cv::circle(displayImage, center, 6, polygonColor, -1, cv::LINE_AA);
    cv::circle(displayImage, center, 6, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
}

std::vector<cv::Point2f> MatchToolDialog::transformPolygonToResult(const std::vector<cv::Point2f>& templatePolygon, const s_SingleTargetMatch& match)
{
    if (templatePolygon.empty()) return std::vector<cv::Point2f>();
    
    // 计算模板多边形的中心点（相对于模板图像）
    cv::Point2f polygonCenter(0, 0);
    for (const auto& point : templatePolygon) {
        polygonCenter += point;
    }
    polygonCenter.x /= templatePolygon.size();
    polygonCenter.y /= templatePolygon.size();
    
    // 计算模板图像的中心点
    cv::Point2f templateImageCenter(
        m_templateImage.cols / 2.0f,
        m_templateImage.rows / 2.0f
    );
    
    // 计算多边形相对于模板图像中心的偏移
    cv::Point2f polygonOffset = polygonCenter - templateImageCenter;
    
    // 应用旋转变换
    double angle = match.dMatchedAngle * CV_PI / 180.0;
    double cosA = cos(angle);
    double sinA = sin(angle);
    
    std::vector<cv::Point2f> transformedPolygon;
    transformedPolygon.reserve(templatePolygon.size());
    
    for (const auto& point : templatePolygon) {
        // 1. 计算点相对于多边形中心的偏移
        cv::Point2f relativePoint = point - polygonCenter;
        
        // 2. 先旋转点
        cv::Point2f rotatedPoint;
        rotatedPoint.x = relativePoint.x * cosA - relativePoint.y * sinA;
        rotatedPoint.y = relativePoint.x * sinA + relativePoint.y * cosA;
        
        // 3. 加上多边形偏移（考虑旋转后的偏移）
        cv::Point2f rotatedOffset;
        rotatedOffset.x = polygonOffset.x * cosA - polygonOffset.y * sinA;
        rotatedOffset.y = polygonOffset.x * sinA + polygonOffset.y * cosA;
        
        // 4. 移动到匹配位置
        cv::Point2f finalPoint = rotatedPoint + rotatedOffset + cv::Point2f(match.ptCenter.x, match.ptCenter.y);
        transformedPolygon.push_back(finalPoint);
    }
    
    return transformedPolygon;
}

 