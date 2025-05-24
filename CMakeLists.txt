cmake_minimum_required(VERSION 3.15)
project(HarmoniaApp)

set(CMAKE_CXX_STANDARD 17)

# Intégration de JUCE localement
add_subdirectory(external/JUCE)

# Tous les fichiers du dossier src
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)

add_executable(HarmoniaApp WIN32 ${SOURCE_FILES})

target_include_directories(HarmoniaApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(HarmoniaApp PRIVATE
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
)
