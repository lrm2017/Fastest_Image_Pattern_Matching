#include <QApplication>
#include <QDir>
#include <QMessageBox>
#include <QFile>
#include "MatchToolDialog.h"

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
    
    // 如果提供了测试图像路径，自动加载
    if (argc > 2) {
        QString sourcePath = QString::fromLocal8Bit(argv[1]);
        QString templatePath = QString::fromLocal8Bit(argv[2]);
        
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