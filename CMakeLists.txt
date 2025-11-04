cmake_minimum_required(VERSION 3.15)
project(HarmoniaApp)

set(CMAKE_CXX_STANDARD 17)

# JUCE
add_subdirectory(external/JUCE)

# --- Ajout de CPR ---
set(CPR_USE_SYSTEM_CURL OFF)
set(CPR_ENABLE_SSL ON)
set(CPR_CURL_USE_LIBPSL OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libs" FORCE)
set(CPR_BUILD_SHARED_LIBS OFF CACHE BOOL "Build CPR as static lib" FORCE)
set(CPR_BUILD_TESTS OFF CACHE BOOL "Don't build CPR tests" FORCE)
add_subdirectory(external/cpr)


# nlohmann/json
add_subdirectory(external/json)

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
    cpr::cpr
    nlohmann_json::nlohmann_json
)
