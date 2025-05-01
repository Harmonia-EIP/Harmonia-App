cmake_minimum_required(VERSION 3.15)
project(MyJUCEApp VERSION 0.1.0)

# Chemin vers vcpkg
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Requis pour JUCE
set(CMAKE_CXX_STANDARD 17)

# Trouver JUCE
find_package(JUCE CONFIG REQUIRED)

juce_add_gui_app(MyJUCEApp
    VERSION "0.1.0"
    COMPANY_NAME "MaBoite"
    PRODUCT_NAME "MyJUCEApp"
)

target_sources(MyJUCEApp PRIVATE
    src/main.cpp
)

target_link_libraries(MyJUCEApp PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_events
)
