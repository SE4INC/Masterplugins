cmake_minimum_required(VERSION 3.22)
project(CRIS_AUDIO_SUITE VERSION 1.0.0 LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG 8.0.6
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

add_subdirectory(CRIS_VOX_TUNER)
add_subdirectory(CRIS_VOX_ENHANCER)
