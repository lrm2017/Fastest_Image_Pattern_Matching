#!/bin/bash

# 构建脚本 for Fastest Image Pattern Matching Qt版本

echo "开始构建 Fastest Image Pattern Matching..."

# 创建构建目录
mkdir -p build
cd build

# 配置CMake
echo "配置CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
echo "编译项目..."
make -j$(nproc)

echo "构建完成！"
echo "可执行文件位置: build/bin/FastestImagePatternMatching" 