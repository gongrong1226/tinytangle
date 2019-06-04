# CMakeList.txt: DAG_cmake 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.4)

#project(sqlite_static)

#源文件
FILE(GLOB_RECURSE sqlite_SOURCES "*.c")
                 
ADD_LIBRARY(sqlite_static STATIC ${sqlite_SOURCES})
#SET_TARGET_PROPERTIES(sqlite_static PROPERTIES OUTPUT_NAME sqlite)
TARGET_LINK_LIBRARIES(sqlite_static)
INSTALL(TARGETS sqlite_static DESTINATION lib)

#增加动态库

#设置头文件搜索目录
include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}
)

# 将源代码添加到此项目的可执行文件。
#add_executable (sqlite_static "sqlite.c" "sqlite.h" "sqliteext.h")

# TODO: 如有需要，请添加测试并安装目标。
