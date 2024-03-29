project(${PluginName} VERSION ${CURRENT_VERSION})

juce_add_plugin(
        "${PluginName}"
        COMPANY_NAME "${PluginVendor}"
        IS_SYNTH FALSE
        NEEDS_MIDI_INPUT FALSE
        NEEDS_MIDI_OUTPUT FALSE
        IS_MIDI_EFFECT FALSE
        VST3_CATEGORIES FX
        AU_MAIN_TYPE kAudioUnitType_Generator
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_PLUGIN_AFTER_BUILD FALSE
        PLUGIN_MANUFACTURER_CODE "${PluginVendorCode}"
        PLUGIN_CODE "${PluginCode}"
        FORMATS VST3 Standalone
        PRODUCT_NAME "${PluginName}"
)

set_target_properties(${PluginName} PROPERTIES CXX_STANDARD 20)

add_subdirectory(Assets)
add_subdirectory(Source)
add_subdirectory(Vendor)


get_property(source_files GLOBAL PROPERTY SRCS)
target_sources(${PluginName} PRIVATE ${source_files})

target_compile_definitions(
        ${PluginName}
        PUBLIC
        JUCE_DISPLAY_SPLASH_SCREEN=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        VERSION="${CURRENT_VERSION}"
)

target_include_directories(${PluginName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Source/)
target_include_directories(${PluginName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/Vendor/)

target_link_libraries(
        ${PluginName} PRIVATE
        juce::juce_audio_utils
        juce::juce_opengl
        juce::juce_dsp
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)


if ((DEFINED ENV{CI}))
    set (env_file "${PROJECT_SOURCE_DIR}/../.env")
    message ("Writing ENV file for CI: ${env_file}")

    # the first call truncates, the rest append
    file(WRITE  "${env_file}" "PROJECT_NAME=${PROJECT_NAME}\n")
    file(APPEND "${env_file}" "PRODUCT_NAME=${PROJECT_NAME}\n")
    file(APPEND "${env_file}" "VERSION=${CURRENT_VERSION}\n")
    file(APPEND "${env_file}" "BUNDLE_ID=${PluginID}\n")
    file(APPEND "${env_file}" "COMPANY_NAME=${PluginVendor}\n")
endif ()