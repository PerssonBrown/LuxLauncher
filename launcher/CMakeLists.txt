cmake_minimum_required(VERSION 3.24)
project(LuxLauncher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_PREFIX_PATH "/opt/Qt/6.4.2/gcc_64/lib/cmake")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/../build/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/../build/release)
add_compile_options(-fPIC)

find_package(Qt6 COMPONENTS Core Gui Widgets Network REQUIRED)

set(CORE_SOURCES
        filesystem.h
        filesystem.cpp
        net/downloader.h
        net/downloader.cpp net/networkrequest.cpp net/networkrequest.h)

add_executable(LuxLauncher ${CORE_SOURCES} main.cpp)

target_link_libraries(LuxLauncher Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)