cmake_minimum_required(VERSION 3.21)
project(bc-gui LANGUAGES C CXX)

# Set target directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin/)

# Find slint installation
find_package(${CMAKE_CURRENT_LIST_DIR}/deps/slint-1.8/lib/cmake/Slint/Slint REQUIRED)

# Find boost installation
find_package(Boost 1.74.0 COMPONENTS filesystem REQUIRED)

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

add_executable(bc-gui 
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/bc.cpp
    ${CMAKE_SOURCE_DIR}/src/logger/logger.cpp
    )

target_link_libraries(bc-gui PRIVATE 
    Slint::Slint
    Boost::filesystem
    ${CMAKE_SOURCE_DIR}/deps/ddc-driver/lib/libemacepl.a)

slint_target_sources(bc-gui ui/app-window.slint)

target_include_directories(bc-gui PUBLIC ${INCLUDEDIRS}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/deps/ddc-driver/include/emacepl
    ${CMAKE_SOURCE_DIR}/deps/ddc-driver/include/ddcacex)