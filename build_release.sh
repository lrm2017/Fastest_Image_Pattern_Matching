#!/bin/bash

# Release构建脚本 for Fastest Image Pattern Matching Qt版本

echo "开始构建 Fastest Image Pattern Matching (Release模式 - 最佳优化)..."

# 创建构建目录
mkdir -p build_release
cd build_release

# 配置CMake为Release模式
echo "配置CMake (Release模式 - 最佳优化)..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译
echo "编译项目 (Release模式 - 最佳优化)..."
make -j$(nproc)

echo "Release构建完成！"
echo "可执行文件位置: build_release/bin/FastestImagePatternMatching"
echo ""
echo "优化选项说明："
echo "-O3: 最高级别优化"
echo "-ffast-math: 快速数学运算"
echo "-funroll-loops: 循环展开"
echo "-ftree-vectorize: 自动向量化"
echo "-march=native: 针对当前CPU优化"
echo "-mtune=native: 针对当前CPU调优"
echo "-mavx2: 启用AVX2指令集"
echo "-msse4.2: 启用SSE4.2指令集"
echo "-mfma: 启用FMA指令集"
echo "-mavx512f: 启用AVX-512指令集"
echo "-flto: 链接时优化"
echo "-pipe: 并行编译" 