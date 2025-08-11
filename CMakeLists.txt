cmake_minimum_required(VERSION 3.16)
project(FastestImagePatternMatching VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找Qt5 (优先使用Qt5，如果没有则尝试Qt6)
find_package(Qt5 COMPONENTS Core Widgets Gui QUIET)
if(Qt5_FOUND)
    set(QT_VERSION 5)
    message(STATUS "Found Qt5: ${Qt5_VERSION}")
else()
    find_package(Qt6 COMPONENTS Core Widgets Gui QUIET)
    if(Qt6_FOUND)
        set(QT_VERSION 6)
        message(STATUS "Found Qt6: ${Qt6_VERSION}")
    else()
        message(FATAL_ERROR "Neither Qt5 nor Qt6 found. Please install Qt5 or Qt6.")
    endif()
endif()

# 查找OpenCV
find_package(OpenCV REQUIRED)

# 查找Threads包 (用于pthread支持)
find_package(Threads REQUIRED)

# 设置编译器特定选项
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug模式 - 启用调试信息，禁用优化
    if(MSVC)
        # Windows MSVC编译器
        add_compile_options(/Od /Zi /MDd)
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Linux/macOS GCC/Clang编译器
        add_compile_options(-O0 -g -DDEBUG)
    endif()
else()
    # Release模式 - 启用最佳优化
    if(MSVC)
        # Windows MSVC编译器 - 最佳优化
        add_compile_options(/O2 /DNDEBUG /GL /Gy /Gw /fp:fast)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            add_compile_options(/arch:AVX2 /arch:AVX512F)
        endif()
        # 链接时优化
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        # Linux/macOS GCC/Clang编译器 - 最佳优化
        add_compile_options(-O3 -DNDEBUG -ffast-math -funroll-loops -ftree-vectorize)
        add_compile_options(-march=native -mtune=native)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            add_compile_options(-mavx2 -msse4.2 -mfma -mavx512f)
        endif()
        # 链接时优化
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")
        # 并行编译
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe")
    endif()
endif()

# 包含目录
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${OpenCV_INCLUDE_DIRS})

# 源文件
set(SOURCES
    src/main.cpp
    src/MatchToolDialog.cpp
    src/TemplateMatcher.cpp
    src/SIMDOptimization.cpp
)

# 头文件
set(HEADERS
    include/MatchToolDialog.h
    include/TemplateMatcher.h
    include/SIMDOptimization.h
    include/DataStructures.h
)

# UI文件
set(UI_FILES
    src/MatchToolDialog.ui
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${UI_FILES})

# 链接库
target_link_libraries(${PROJECT_NAME} 
    ${OpenCV_LIBS}
    Qt${QT_VERSION}::Core
    Qt${QT_VERSION}::Widgets
    Qt${QT_VERSION}::Gui
    Threads::Threads
)

# 设置Qt自动生成的文件
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
)

# 复制资源文件到输出目录
# file(COPY ${CMAKE_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR}/bin)

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装资源文件
# install(DIRECTORY resources/ DESTINATION share/${PROJECT_NAME}/resources)

# 打印配置信息
message(STATUS "Qt Version: ${QT_VERSION}")
message(STATUS "OpenCV Version: ${OpenCV_VERSION}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
