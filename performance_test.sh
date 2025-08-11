#!/bin/bash

# 性能测试脚本 - 比较Debug和Release版本的执行时间

echo "=== 性能测试脚本 ==="
echo ""

# 检查可执行文件是否存在
if [ ! -f "build_debug/bin/FastestImagePatternMatching" ]; then
    echo "错误: Debug版本不存在，请先运行 ./build_debug.sh"
    exit 1
fi

if [ ! -f "build_release/bin/FastestImagePatternMatching" ]; then
    echo "错误: Release版本不存在，请先运行 ./build_release.sh"
    exit 1
fi

echo "文件大小比较："
echo "Debug版本: $(ls -lh build_debug/bin/FastestImagePatternMatching | awk '{print $5}')"
echo "Release版本: $(ls -lh build_release/bin/FastestImagePatternMatching | awk '{print $5}')"
echo ""

# 检查是否有测试图像
if [ ! -d "Test Images" ]; then
    echo "警告: 未找到Test Images目录，请确保有测试图像"
    echo "您需要手动运行程序进行性能测试"
    echo ""
    echo "使用方法："
    echo "1. 运行Debug版本: ./build_debug/bin/FastestImagePatternMatching"
    echo "2. 运行Release版本: ./build_release/bin/FastestImagePatternMatching"
    echo "3. 分别记录执行时间进行比较"
    exit 0
fi

echo "找到测试图像目录，开始性能测试..."
echo ""

# 测试图像文件
SOURCE_IMAGE="Test Images/Src1.bmp"
TEMPLATE_IMAGE="Test Images/Dst1.bmp"

if [ ! -f "$SOURCE_IMAGE" ] || [ ! -f "$TEMPLATE_IMAGE" ]; then
    echo "警告: 未找到测试图像文件，请手动测试"
    echo "建议使用 Test Images/ 目录中的图像进行测试"
    exit 0
fi

echo "使用测试图像："
echo "源图像: $SOURCE_IMAGE"
echo "模板图像: $TEMPLATE_IMAGE"
echo ""

echo "=== 性能测试结果 ==="
echo "注意: 由于这是GUI程序，无法直接测量执行时间"
echo "请手动运行两个版本并比较执行时间："
echo ""
echo "1. Debug版本 (4.1M):"
echo "   ./build_debug/bin/FastestImagePatternMatching"
echo ""
echo "2. Release版本 (280K):"
echo "   ./build_release/bin/FastestImagePatternMatching"
echo ""
echo "预期性能提升："
echo "- 文件大小减少: ~93% (从4.1M减少到280K)"
echo "- 执行速度提升: 预计2-10倍 (取决于算法复杂度)"
echo "- 内存使用减少: 预计30-50%"
echo ""
echo "测试建议："
echo "1. 使用相同的测试图像"
echo "2. 记录匹配操作的执行时间"
echo "3. 比较状态栏显示的执行时间"
echo "4. 观察内存使用情况" 