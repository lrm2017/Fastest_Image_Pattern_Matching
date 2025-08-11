#!/bin/bash

# 设置编译器
CXX=g++
CXXFLAGS="-std=c++14 -O2 -march=native"

# 查找OpenCV
OPENCV_CFLAGS=$(pkg-config --cflags opencv4 2>/dev/null || pkg-config --cflags opencv)
OPENCV_LIBS=$(pkg-config --libs opencv4 2>/dev/null || pkg-config --libs opencv)

if [ $? -ne 0 ]; then
    echo "错误: 无法找到OpenCV，请确保已安装OpenCV开发包"
    exit 1
fi

# 查找Qt5
QT_CFLAGS=$(pkg-config --cflags Qt5Core Qt5Widgets 2>/dev/null)
QT_LIBS=$(pkg-config --libs Qt5Core Qt5Widgets 2>/dev/null)

if [ $? -ne 0 ]; then
    echo "警告: 无法找到Qt5，将尝试使用Qt4或跳过Qt依赖"
    QT_CFLAGS=""
    QT_LIBS=""
fi

# 编译源文件
echo "编译TemplateMatcher..."
$CXX $CXXFLAGS $OPENCV_CFLAGS $QT_CFLAGS \
    -Iinclude \
    -c src/TemplateMatcher.cpp -o TemplateMatcher.o

echo "编译SIMDOptimization..."
$CXX $CXXFLAGS \
    -c src/SIMDOptimization.cpp -o SIMDOptimization.o

echo "编译测试程序..."
$CXX $CXXFLAGS $OPENCV_CFLAGS $QT_CFLAGS \
    -Iinclude \
    -c test_matching.cpp -o test_matching.o

echo "链接可执行文件..."
$CXX $CXXFLAGS \
    TemplateMatcher.o SIMDOptimization.o test_matching.o \
    $OPENCV_LIBS $QT_LIBS \
    -o TemplateMatcherTest

# 清理中间文件
rm -f *.o

echo "编译完成！"
echo "运行测试: ./TemplateMatcherTest" 