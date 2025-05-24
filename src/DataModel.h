#include "JuceHeader.h"

class DataModel
{
public:
    juce::String waveform;
    double frequency = 440.0;
    double volume = 0.8;

    void saveToJson()
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();
        obj->setProperty("waveform", waveform);
        obj->setProperty("frequency", frequency);
        obj->setProperty("volume", volume);

        juce::var json(obj);

        juce::File file = juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
                            .getChildFile("generated_patch.json");
        file.replaceWithText(juce::JSON::toString(json, true));
    }
};
