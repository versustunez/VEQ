add_subdirectory(Core)
add_subdirectory(Events)
add_subdirectory(GUI)
add_subdirectory(Editor)
add_sources(
        Audio/AutoGain.cpp
        Audio/FFT.cpp
        BandListener.cpp
        PluginEditor.cpp
        PluginProcessor.cpp
        Filter.cpp
        UI.cpp
        Utils/UI.cpp
)
