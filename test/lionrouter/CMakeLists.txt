cmake_minimum_required(VERSION 3.10)
project(LionRouterTest)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置默认构建类型为 Debug
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用调试信息
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")


# 查找 Google Test
find_package(GTest REQUIRED)

# 添加 LionRouter 的源文件
set(LIONROUTER_SOURCES
    ../../lib/lionrouter.cpp
)

# 添加测试文件
set(TEST_SOURCES
    test_lionrouter.cpp
)

# 创建可执行文件
add_executable(test_lionrouter ${TEST_SOURCES} ${LIONROUTER_SOURCES})

# 包含 LionRouter 的头文件目录
target_include_directories(test_lionrouter PRIVATE ../../include)

# 包含本地的 curl 头文件目录
# 假设 curl 的头文件位于 ../../curl/include 目录下
target_include_directories(test_lionrouter PRIVATE ../../curl/include)

# 链接本地的 curl 库
# 假设 curl 的库文件位于 ../../curl/lib 目录下，库文件名为 libcurl.a 或 libcurl.so
target_link_directories(test_lionrouter PRIVATE ../../curl/lib)
target_link_libraries(test_lionrouter PRIVATE curl)

# 链接 Google Test 和 pthread
target_link_libraries(test_lionrouter
    PRIVATE
    GTest::GTest
    GTest::Main
    pthread
)

# 明确 C++ 标准
target_compile_features(test_lionrouter PRIVATE cxx_std_17)