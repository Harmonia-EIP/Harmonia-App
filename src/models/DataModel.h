#pragma once
#include "JuceHeader.h"

class DataModel
{
public:
    juce::String waveform;
    double frequency = 440.0;
    double volume = 0.8;

    // ADSR
    double attack = 0.1;
    double decay = 0.1;
    double sustain = 0.8;
    double release = 0.5;

    // Filtre
    juce::String filterType = "Low Pass";
    double cutoff = 1000.0;
    double resonance = 1.0;

    // Prompt
    juce::String prompt;

    void saveToJson()
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("waveform", waveform);
        obj->setProperty("frequency", frequency);
        obj->setProperty("volume", volume);

        obj->setProperty("attack", attack);
        obj->setProperty("decay", decay);
        obj->setProperty("sustain", sustain);
        obj->setProperty("release", release);

        obj->setProperty("filterType", filterType);
        obj->setProperty("cutoff", cutoff);
        obj->setProperty("resonance", resonance);

        obj->setProperty("prompt", prompt);

        juce::var json(obj);

        juce::File outputFolder = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                                .getParentDirectory()
                                .getChildFile("../../generated_presets");

if (!outputFolder.exists())
    outputFolder.createDirectory();

juce::File file = outputFolder.getChildFile("generated_patch.json");
file.replaceWithText(juce::JSON::toString(json, true));


    }
};
