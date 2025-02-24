cmake_minimum_required(VERSION 3.30)
project(Model)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH rootDir)
cmake_path(GET rootDir PARENT_PATH rootDir)
set(AssimpDir "D:/libraries/assimp_5_4_0")

# 告诉编译器去哪里查找头文件和库文件
include_directories(${rootDir}/include)
include_directories(${rootDir}/source)
link_directories(${rootDir}/lib)

include_directories(${AssimpDir}/include)
message(STATUS "~~~~~${AssimpDir}/include~~~~~~")
link_directories(${AssimpDir}/build/bin)
message(STATUS "~~~~~${AssimpDir}/build/bin~~~~~~")

add_compile_options(-Wall -std=c++17)
add_executable(model model.hpp mesh.hpp global.h main.cc)# 生成可执行文件

target_link_libraries(model libglad.a libglfw3.a libassimp-5.dll)# 链接库，这里使用的是静态库