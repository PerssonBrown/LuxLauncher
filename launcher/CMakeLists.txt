cmake_minimum_required(VERSION 3.24)
project(LuxLauncher)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/../build/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/../build/release)
add_compile_options(-fPIC)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)

set(CORE_SOURCES
        FileSystem.h
        FileSystem.cpp
        net/Downloader.h
        net/Downloader.cpp
        net/NetworkRequest.h
        net/NetworkRequest.cpp
        account/AccountManager.h
        account/AccountManager.cpp )

add_executable(LuxLauncher ${CORE_SOURCES} main.cpp)

target_link_libraries(LuxLauncher PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)