cmake_minimum_required(VERSION 3.20)
project(ADVANCED_LIGHTING)

set(myClassDir "../MyClass")
set(packagesDir "$ENV{HOME}") #直接获取系统环境变量
set(packagesDir "${packagesDir}/packages")
set(libDir "${packagesDir}/lib")
# message(STATUS "~~~~~${packagesDir}~~~~~~")

# 头文件目录
include_directories(${packagesDir}/include)
include_directories(${myClassDir}) #自定义类

# 编译
add_compile_options(-Wall -std=c++20)
add_executable(normalMapping global.h main.cc)# 生成可执行文件

# 指明编译阶段库文件
target_link_libraries(normalMapping
${libDir}/libglfw.so 
${libDir}/libglad.a 
${libDir}/libassimp.so)

# 指明运行阶段的库文件路径
set(CMAKE_INSTALL_RPATH "${libDir}")
