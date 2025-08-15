#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QFile>
#include <QCommandLineParser>
#include "MatchToolDialog.h"
#include "CameraPreviewDialog.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Fastest Image Pattern Matching");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("OpenCV Pattern Matching");
    
    // 创建主对话框
    MatchToolDialog dialog;
    dialog.show();
    
    // 命令行参数解析
    QCommandLineParser parser;
    parser.setApplicationDescription("Fastest Image Pattern Matching Tool");
    parser.addHelpOption();
    parser.addVersionOption();
    
    // 添加摄像头测试选项
    QCommandLineOption cameraOption(QStringList() << "c" << "camera", 
                                   "测试摄像头功能");
    parser.addOption(cameraOption);
    
    // 添加图像路径选项
    QCommandLineOption sourceOption(QStringList() << "s" << "source", 
                                   "源图像路径", "path");
    parser.addOption(sourceOption);
    
    QCommandLineOption templateOption(QStringList() << "t" << "template", 
                                    "模板图像路径", "path");
    parser.addOption(templateOption);
    
    parser.process(app);
    
    // 如果指定了摄像头测试，显示摄像头预览对话框
    if (parser.isSet(cameraOption)) {
        CameraPreviewDialog cameraDialog;
        cameraDialog.show();
        return app.exec();
    }
    
    // 如果提供了测试图像路径，自动加载
    if (parser.isSet(sourceOption) && parser.isSet(templateOption)) {
        QString sourcePath = parser.value(sourceOption);
        QString templatePath = parser.value(templateOption);
        
        if (QFile::exists(sourcePath) && QFile::exists(templatePath)) {
            dialog.loadSourceImage(sourcePath);
            dialog.loadTemplateImage(templatePath);
            QMessageBox::information(&dialog, "测试模式", 
                QString("已自动加载测试图像:\n源图像: %1\n模板图像: %2")
                .arg(sourcePath).arg(templatePath));
        }
    }
    
    return app.exec();
} 