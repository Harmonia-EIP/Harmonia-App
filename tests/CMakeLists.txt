include(FetchContent)
FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.7.1)
FetchContent_MakeAvailable(Catch2)

# Mêmes sources que le plugin (PluginProcessor, backend, synth...)
add_executable(Harmonia_Tests EXCLUDE_FROM_ALL
    main.cpp
    ProcessorTests.cpp
    ProcessorAudioTests.cpp
    ParametersTests.cpp
    PresetsTests.cpp
    UiTests.cpp
    ${PLUGIN_SOURCES})

target_include_directories(Harmonia_Tests PRIVATE
    ${CMAKE_SOURCE_DIR}/src)

# Récupère les définitions JucePlugin_* du plugin
target_compile_definitions(Harmonia_Tests PRIVATE
    $<TARGET_PROPERTY:Harmonia,COMPILE_DEFINITIONS>
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_MODAL_LOOPS_PERMITTED=1)

target_link_libraries(Harmonia_Tests PRIVATE
    HarmoniaAssets
    cpr::cpr
    nlohmann_json::nlohmann_json
    juce::juce_audio_utils
    juce::juce_audio_processors
    juce::juce_audio_devices
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_core
    juce::juce_dsp
    Catch2::Catch2)   # pas WithMain : on a notre propre main

# Étape 1 : compiler l'exécutable de tests
add_test(NAME build_tests
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target Harmonia_Tests)
set_tests_properties(build_tests PROPERTIES FIXTURES_SETUP harmonia_build)

# Étape 2 : lancer les tests
add_test(NAME unit_tests COMMAND Harmonia_Tests)
set_tests_properties(unit_tests PROPERTIES FIXTURES_REQUIRED harmonia_build)
# Moteur v3 seul (C++17 pur, sans JUCE ni réseau)
add_executable(HarmoniaEngine_Tests EXCLUDE_FROM_ALL
    engine/EngineTests.cpp
    ${CMAKE_SOURCE_DIR}/src/engine/HarmoniaEngineV3.cpp)

add_test(NAME build_engine_tests
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target HarmoniaEngine_Tests)
set_tests_properties(build_engine_tests PROPERTIES FIXTURES_SETUP harmonia_engine_build)

add_test(NAME engine_tests COMMAND HarmoniaEngine_Tests)
set_tests_properties(engine_tests PROPERTIES FIXTURES_REQUIRED harmonia_engine_build)
