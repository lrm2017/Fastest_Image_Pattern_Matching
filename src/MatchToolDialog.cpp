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
    , m_sourceImageLabel(nullptr)
    , m_templateImageLabel(nullptr)
    , m_executionTimeLabel(nullptr)
    , m_lastSourceImagePath("")
    , m_lastTemplateImagePath("")
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
    
    // 启用拖动平移
    ui->sourceView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->sourceView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    
    // 安装事件过滤器以处理鼠标滚轮缩放
    ui->sourceView->viewport()->installEventFilter(this);
    
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
        } else {
            ui->templateView->setTransform(QTransform::fromScale(scale, scale));
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
        
        // 绘制虚线矩形框
        cv::Point2d ptLT(result.ptLT.x, result.ptLT.y);
        cv::Point2d ptRT(result.ptRT.x, result.ptRT.y);
        cv::Point2d ptLB(result.ptLB.x, result.ptLB.y);
        cv::Point2d ptRB(result.ptRB.x, result.ptRB.y);
        cv::Point2d ptC(result.ptCenter.x, result.ptCenter.y);
        
        // 绘制实线矩形（更容易看到）
        cv::line(displayImage, ptLT, ptLB, cv::Scalar(0, 255, 0), 2);
        cv::line(displayImage, ptLB, ptRB, cv::Scalar(0, 255, 0), 2);
        cv::line(displayImage, ptRB, ptRT, cv::Scalar(0, 255, 0), 2);
        cv::line(displayImage, ptRT, ptLT, cv::Scalar(0, 255, 0), 2);
        
        // 绘制角落标记
        cv::Point2d ptDis1, ptDis2;
        // 使用匹配结果的矩形大小来计算角落标记
        double rectWidth = cv::norm(ptRT - ptLT);
        double rectHeight = cv::norm(ptLB - ptLT);
        
        if (rectWidth > rectHeight) {
            ptDis1 = (ptLB - ptLT) / 3.0;
            ptDis2 = (ptRT - ptLT) / 3.0 * (rectHeight / rectWidth);
        } else {
            ptDis1 = (ptLB - ptLT) / 3.0 * (rectWidth / rectHeight);
            ptDis2 = (ptRT - ptLT) / 3.0;
        }
        
        // 绘制角落线条
        cv::line(displayImage, ptLT, ptLT + ptDis1/2, cv::Scalar(0, 255, 0), 1);
        cv::line(displayImage, ptLT, ptLT + ptDis2/2, cv::Scalar(0, 255, 0), 1);
        cv::line(displayImage, ptRT, ptRT + ptDis1/2, cv::Scalar(0, 255, 0), 1);
        cv::line(displayImage, ptRT, ptRT - ptDis2/2, cv::Scalar(0, 255, 0), 1);
        cv::line(displayImage, ptRB, ptRB - ptDis1/2, cv::Scalar(0, 255, 0), 1);
        cv::line(displayImage, ptRB, ptRB - ptDis2/2, cv::Scalar(0, 255, 0), 1);
        cv::line(displayImage, ptLB, ptLB - ptDis1/2, cv::Scalar(0, 255, 0), 1);
        cv::line(displayImage, ptLB, ptLB + ptDis2/2, cv::Scalar(0, 255, 0), 1);
        
        // 绘制中心圆点（更容易看到）
        cv::circle(displayImage, ptC, 5, cv::Scalar(0, 0, 255), -1);
        
        // 绘制序号（更大更明显）
        std::string text = std::to_string(i);
        cv::putText(displayImage, text, cv::Point(ptLT.x + 10, ptLT.y + 20), 
                   cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 255, 0), 2);
        
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
    // 绘制虚线
    double dx = ptEnd.x - ptStart.x;
    double dy = ptEnd.y - ptStart.y;
    double length = sqrt(dx*dx + dy*dy);
    
    if (length < 1) return;
    
    int dashLength = 5;
    int gapLength = 3;
    double step = dashLength + gapLength;
    
    cv::Point2d direction(dx/length, dy/length);
    cv::Point2d current = ptStart;
    
    while (cv::norm(current - ptStart) < length) {
        cv::Point2d end = current + direction * dashLength;
        if (cv::norm(end - ptStart) > length) {
            end = ptEnd;
        }
        cv::line(matDraw, current, end, color1, 1);
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
    if (obj == ui->sourceView->viewport() && event->type() == QEvent::Wheel) {
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
    
    // 更新状态栏显示当前缩放比例（临时显示）
    ui->statusBar->showMessage(QString("缩放比例: %1%").arg(m_currentScale * 100, 0, 'f', 1), 2000);
}

 