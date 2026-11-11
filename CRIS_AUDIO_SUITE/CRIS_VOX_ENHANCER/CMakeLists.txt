juce_add_plugin(CRIS_VOX_ENHANCER
    COMPANY_NAME "CRIS Audio"
    BUNDLE_ID "com.crisaudio.voxenhancer"
    PLUGIN_MANUFACTURER_CODE Cris
    PLUGIN_CODE CvE1
    FORMATS VST3 Standalone
    PRODUCT_NAME "CRIS VOX ENHANCER"
    DESCRIPTION "Vocal tone and harmonic enhancer"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)
juce_generate_juce_header(CRIS_VOX_ENHANCER)
target_sources(CRIS_VOX_ENHANCER PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
)
target_compile_features(CRIS_VOX_ENHANCER PRIVATE cxx_std_17)
target_compile_definitions(CRIS_VOX_ENHANCER PUBLIC JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(CRIS_VOX_ENHANCER PRIVATE
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC juce::juce_recommended_config_flags
           juce::juce_recommended_lto_flags
           juce::juce_recommended_warning_flags
)
