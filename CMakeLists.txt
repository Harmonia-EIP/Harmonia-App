cmake_minimum_required(VERSION 3.15)

project(Harmonia VERSION 1.0.0)

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
# PLUGIN JUCE
# =========================
juce_add_plugin(HarmoniaPlugin
    VERSION 1.0.0
    COMPANY_NAME "Harmonia"

    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT TRUE

    PLUGIN_MANUFACTURER_CODE Hrmn
    PLUGIN_CODE HApp

    FORMATS VST3 Standalone

    PRODUCT_NAME "HarmoniaPlugin"
)

target_compile_definitions(HarmoniaPlugin PUBLIC
    JUCE_VST3_CAN_REPLACE_VST2=0
)

juce_generate_juce_header(HarmoniaPlugin)

# =========================
# SOURCES
# =========================
file(GLOB_RECURSE PLUGIN_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
    src/*.h
)

target_sources(HarmoniaPlugin PRIVATE
    ${PLUGIN_SOURCES}
)

target_include_directories(HarmoniaPlugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(HarmoniaPlugin PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json

    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_devices
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_core
    juce::juce_dsp
)