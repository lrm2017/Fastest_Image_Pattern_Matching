#!/bin/bash

echo "=== 调试配置测试脚本 ==="

# 检查必要的工具
echo "1. 检查必要工具..."
if command -v gdb &> /dev/null; then
    echo "✓ GDB已安装"
else
    echo "✗ GDB未安装，请安装: sudo apt-get install gdb"
    exit 1
fi

if command -v cmake &> /dev/null; then
    echo "✓ CMake已安装"
else
    echo "✗ CMake未安装，请安装: sudo apt-get install cmake"
    exit 1
fi

if command -v make &> /dev/null; then
    echo "✓ Make已安装"
else
    echo "✗ Make未安装，请安装: sudo apt-get install make"
    exit 1
fi

# 构建调试版本
echo "2. 构建调试版本..."
if [ -f "./build_debug.sh" ]; then
    chmod +x ./build_debug.sh
    ./build_debug.sh
else
    echo "✗ build_debug.sh不存在"
    exit 1
fi

# 检查可执行文件
echo "3. 检查可执行文件..."
if [ -f "./build_debug/bin/FastestImagePatternMatching" ]; then
    echo "✓ 可执行文件已生成"
    
    # 检查是否包含调试信息
    if file "./build_debug/bin/FastestImagePatternMatching" | grep -q "not stripped"; then
        echo "✓ 包含调试信息"
    else
        echo "✗ 不包含调试信息，请检查CMakeLists.txt配置"
    fi
else
    echo "✗ 可执行文件未生成"
    exit 1
fi

# 测试GDB连接
echo "4. 测试GDB连接..."
echo "quit" | gdb "./build_debug/bin/FastestImagePatternMatching" > /dev/null 2>&1
if [ $? -eq 0 ]; then
    echo "✓ GDB可以正常加载程序"
else
    echo "✗ GDB无法加载程序"
    exit 1
fi

echo "=== 调试配置测试完成 ==="
echo ""
echo "现在您可以："
echo "1. 在VS Code中按F5开始调试"
echo "2. 使用命令行: gdb build_debug/bin/FastestImagePatternMatching"
echo "3. 查看DEBUG_GUIDE.md获取详细说明" 