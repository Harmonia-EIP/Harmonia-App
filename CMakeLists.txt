cmake_minimum_required(VERSION 3.15)
project(HarmoniaApp VERSION 0.1.0)

# Chemin vers vcpkg
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "")

# Requis pour JUCE
set(CMAKE_CXX_STANDARD 17)

# Trouver JUCE
find_package(JUCE CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)

juce_add_gui_app(HarmoniaApp
    VERSION "0.1.0"
    COMPANY_NAME "Harmonia"
    PRODUCT_NAME "HarmoniaApp"
)

# Recursive sur tout les fichiers
file(GLOB SOURCE_FILES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)

target_sources(HarmoniaApp PRIVATE ${SOURCE_FILES})

target_link_libraries(HarmoniaApp PRIVATE
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_events
    nlohmann_json::nlohmann_json
)
