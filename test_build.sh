#!/bin/bash

echo "测试构建 Fastest Image Pattern Matching..."

# 清理之前的构建
rm -rf build
mkdir -p build
cd build

# 配置CMake
echo "配置CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
echo "编译项目..."
make -j$(nproc)

echo "构建完成！" 