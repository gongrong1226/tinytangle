# CMakeList.txt: dag_cmake1 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
FILE(GLOB_RECURSE tinytangle_SOURCES "*.cpp")

#SET(CMAKE_CXX_FLAGS "-Wdeprecated-declarations ${CMAKE_CXX_FLAGS}")
add_compile_options("-Wdeprecated-declarations")

# 将源代码添加到此项目的可执行文件。
ADD_EXECUTABLE(tinytangle ${tinytangle_SOURCES})


IF(WIN32)
    TARGET_LINK_LIBRARIES(tinytangle ${Boost_LIBRARIES} ${json_LIBRARY} ${sqlite_LIBRARY})
ELSEIF(ENABLE_SHARED_LIBS)
    TARGET_LINK_LIBRARIES(tinytangle ${Boost_LIBRARIES} ${json_LIBRARY} ${sqlite_LIBRARY} cryptopp)
ELSE()
    TARGET_LINK_LIBRARIES(tinytangle ${Boost_LIBRARIES} ${json_LIBRARY} ${sqlite_LIBRARY} cryptopp dl)
ENDIF()


INSTALL(TARGETS tinytangle DESTINATION bin)
# TODO: 如有需要，请添加测试并安装目标。
