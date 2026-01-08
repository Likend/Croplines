cmake_minimum_required(VERSION 3.21)
project(Croplines)

add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    set(CMAKE_EXE_LINKER_FLAGS "-static-libstdc++ -static-libgcc")
    set(CMAKE_CXX_FLAGS_RELEASE "-s")
endif()

if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
    set(wxWidgets_USE_DEBUG ON)
else()
    set(wxWidgets_USE_STATIC ON)
endif()
find_package(wxWidgets REQUIRED aui core base gl)
find_package(Eigen3 REQUIRED) # required by opencv
find_package(OpenCV REQUIRED)
find_package(OpenMP REQUIRED)
include(${wxWidgets_USE_FILE})
include_directories(${OpenCV_INCLUDE_DIRS})

file(GLOB_RECURSE SRCS src/*.cpp)
if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
set (WIN32_RESOURCES src/resource.rc)
endif()

if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Debug"))
add_executable(Croplines  ${SRCS} ${WIN32_RESOURCES})
else()
add_executable(Croplines WIN32 ${SRCS} ${WIN32_RESOURCES})
endif()
target_include_directories(Croplines PRIVATE src)
target_link_libraries(Croplines ${wxWidgets_LIBRARIES})
target_link_libraries(Croplines ${OpenCV_LIBS})
target_link_libraries(Croplines opengl32)


if(CMAKE_BUILD_TYPE AND (CMAKE_BUILD_TYPE STREQUAL "Release"))
    target_link_libraries(Croplines OpenMP::OpenMP_CXX -static-libgcc
            -static-libstdc++
            -Wl,-Bstatic -lgomp -Wl,-Bdynamic)
    target_link_libraries(Croplines -Wl,-Bstatic stdc++ pthread -Wl,-Bdynamic) # 解决libwinpthread-1.dll依赖问题
endif()
