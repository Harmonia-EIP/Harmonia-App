cmake_minimum_required(VERSION 3.15)

project(HarmoniaApp VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

# =========================
# JUCE
# =========================
add_subdirectory(external/JUCE)

# =========================
# CPR (HTTP)
# =========================
set(CPR_USE_SYSTEM_CURL OFF)
set(CPR_ENABLE_SSL ON)
set(CPR_CURL_USE_LIBPSL OFF)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CPR_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CPR_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/cpr)

# =========================
# JSON
# =========================
add_subdirectory(external/json)

# =========================
# SHARED CODE (optionnel mais propre)
# =========================
file(GLOB_RECURSE SHARED_SOURCES CONFIGURE_DEPENDS
    src/shared/*.cpp
    src/shared/*.h
)

add_library(HarmoniaCore ${SHARED_SOURCES})

target_include_directories(HarmoniaCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(HarmoniaCore PUBLIC
    cpr::cpr
    nlohmann_json::nlohmann_json

    juce::juce_core
    juce::juce_events
    juce::juce_data_structures
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_dsp
)

# =========================
# PLUGIN JUCE (VST3 + Standalone)
# =========================
juce_add_plugin(HarmoniaPlugin
    VERSION 1.0.0
    COMPANY_NAME "Harmonia"

    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE

    PLUGIN_MANUFACTURER_CODE Hrmn
    PLUGIN_CODE HApp

    FORMATS VST3 Standalone

    PRODUCT_NAME "HarmoniaPlugin"
)

target_compile_definitions(HarmoniaPlugin PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_STANDALONE_APPLICATION=1
)

# IMPORTANT
juce_generate_juce_header(HarmoniaPlugin)

# fichiers plugin uniquement
target_sources(HarmoniaPlugin PRIVATE
    src/plugin/PluginProcessor.cpp
    src/plugin/PluginEditor.cpp
    src/plugin/PluginAppWrapper.cpp
)

target_link_libraries(HarmoniaPlugin PRIVATE
    HarmoniaCore

    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_devices
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_core
)

# =========================
# APP CUSTOM (EXE)
# =========================
add_executable(HarmoniaApp WIN32
    src/app/main.cpp
    src/app/StandaloneAppWrapper.cpp
    src/app/StandaloneApp.cpp
    src/app/StandaloneWindow.cpp
    icon.rc
)

target_include_directories(HarmoniaApp PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(HarmoniaApp PRIVATE
    HarmoniaCore
)

# =========================
# COPY VST3 TO INSTALLER (FIXED PATH)
# =========================
add_custom_command(TARGET HarmoniaPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying VST3 to installer..."

    COMMAND ${CMAKE_COMMAND} -E rm -rf
        "${CMAKE_SOURCE_DIR}/installer/HarmoniaPlugin.vst3"

    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_BINARY_DIR}/HarmoniaPlugin_artefacts/$<CONFIG>/VST3/HarmoniaPlugin.vst3"
        "${CMAKE_SOURCE_DIR}/installer/HarmoniaPlugin.vst3"
)