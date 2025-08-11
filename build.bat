@echo off
REM Windows构建脚本 for Fastest Image Pattern Matching Qt版本

echo 开始构建 Fastest Image Pattern Matching...

REM 创建构建目录
if not exist build mkdir build
cd build

REM 配置CMake
echo 配置CMake...
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release

REM 编译
echo 编译项目...
cmake --build . --config Release --parallel

echo 构建完成！
echo 可执行文件位置: build\bin\Release\FastestImagePatternMatching.exe
pause 