cmake_minimum_required(VERSION 3.30)

project(SPHERE)

#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED True)

# get_filename_component(rootDir ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
# get_filename_component(rootDir ${rootDir} DIRECTORY)
# message("${rootDir}\n")

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH rootDir)
cmake_path(GET rootDir PARENT_PATH rootDir)

#message("${rootDir#}\n")

# 告诉编译器去哪里查找头文件和库文件
include_directories(${rootDir}/include)
include_directories(${rootDir}/source)
link_directories(${rootDir}/lib)

add_compile_options(-Wall -std=c++20)
add_executable(sphere sphere.cc)# 生成可执行文件

target_link_libraries(sphere libglad.a libglfw3.a)# 链接库，这里使用的是静态库




