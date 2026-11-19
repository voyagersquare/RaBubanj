cmake_minimum_required(VERSION 3.22)
project(RaBubanj VERSION 0.6.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

include(CTest)
add_executable(drum_engine_tests tests/DrumEngineTests.cpp)
target_include_directories(drum_engine_tests PRIVATE Source)
add_test(NAME drum_engine COMMAND drum_engine_tests)
add_executable(eq_tests tests/EqTests.cpp)
target_include_directories(eq_tests PRIVATE Source)
add_test(NAME parametric_eq COMMAND eq_tests)
add_executable(factory_preset_tests tests/FactoryPresetTests.cpp)
target_include_directories(factory_preset_tests PRIVATE Source)
add_test(NAME factory_presets COMMAND factory_preset_tests)

option(RABUBANJ_BUILD_PLUGIN "Build the JUCE instrument" ON)
if(RABUBANJ_BUILD_PLUGIN)
  include(FetchContent)
  if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
  endif()
  FetchContent_Declare(JUCE
    URL https://codeload.github.com/juce-framework/JUCE/tar.gz/51a8a6d7aeae7326956d747737ccf1575e61e209
    URL_HASH SHA256=20b740351cc030086951d87614bc08b42d072f1e4db68db1ef1d988e66dbbdbe)
  FetchContent_MakeAvailable(JUCE)
  juce_add_plugin(RaBubanj
    COMPANY_NAME "Voyagersquare"
    BUNDLE_ID "com.voyagersquare.rabubanj"
    PLUGIN_MANUFACTURER_CODE Vysq
    PLUGIN_CODE Rbub
    PRODUCT_NAME "RaBubanj"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES Instrument Drum
    FORMATS VST3 Standalone)
  juce_generate_juce_header(RaBubanj)
  target_sources(RaBubanj PRIVATE Source/PluginProcessor.cpp Source/PluginEditor.cpp Source/EqPanel.cpp Source/SampleLoader.cpp)
  target_compile_definitions(RaBubanj PUBLIC
    JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_REPORT_APP_USAGE=0)
  target_link_libraries(RaBubanj PRIVATE
    juce::juce_audio_utils juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags juce::juce_recommended_warning_flags)
  add_executable(processor_tests tests/ProcessorTests.cpp)
  target_include_directories(processor_tests PRIVATE Source "${juce_SOURCE_DIR}/modules")
  target_compile_definitions(processor_tests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
  target_link_libraries(processor_tests PRIVATE RaBubanj)
  add_test(NAME processor COMMAND processor_tests)
  add_executable(mixer_tests tests/MixerTests.cpp)
  target_include_directories(mixer_tests PRIVATE Source "${juce_SOURCE_DIR}/modules")
  target_compile_definitions(mixer_tests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
  target_link_libraries(mixer_tests PRIVATE RaBubanj)
  add_test(NAME mixer_routing_and_editor COMMAND mixer_tests)
  add_executable(sample_import_tests tests/SampleImportTests.cpp)
  target_include_directories(sample_import_tests PRIVATE Source "${juce_SOURCE_DIR}/modules")
  target_compile_definitions(sample_import_tests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
  target_link_libraries(sample_import_tests PRIVATE RaBubanj)
  add_test(NAME sample_import_and_recall COMMAND sample_import_tests)
  add_executable(channel_colour_tests tests/ChannelColourTests.cpp)
  target_include_directories(channel_colour_tests PRIVATE Source "${juce_SOURCE_DIR}/modules")
  target_compile_definitions(channel_colour_tests PRIVATE JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1)
  target_link_libraries(channel_colour_tests PRIVATE RaBubanj)
  add_test(NAME channel_colour_and_spectrum COMMAND channel_colour_tests)
endif()
