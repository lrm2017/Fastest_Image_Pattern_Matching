#!/bin/bash

# 调试构建脚本 for Fastest Image Pattern Matching Qt版本

echo "开始构建 Fastest Image Pattern Matching (Debug模式)..."

# 创建构建目录
mkdir -p build_debug
cd build_debug

# 配置CMake为Debug模式
echo "配置CMake (Debug模式)..."
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 编译
echo "编译项目 (Debug模式)..."
make -j$(nproc)

echo "调试构建完成！"
echo "可执行文件位置: build_debug/bin/FastestImagePatternMatching"
echo "现在可以使用gdb进行调试:"
echo "  gdb build_debug/bin/FastestImagePatternMatching"
echo "或者在IDE中打开项目进行调试" 