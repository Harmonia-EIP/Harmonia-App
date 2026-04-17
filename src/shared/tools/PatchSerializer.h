#pragma once
#include "../parameters/HarmoniaParams.h"
#include <optional>

namespace PatchSerializer
{
    // ===== ENUM → STRING =====
    inline juce::String filterToString(FilterType type)
    {
        switch (type)
        {
            case FilterType::LOWPASS:  return "Low Pass";
            case FilterType::HIGHPASS: return "High Pass";
            case FilterType::BANDPASS: return "Band Pass";
        }
        jassertfalse;
        return "Low Pass";
    }

    inline FilterType filterFromString(const juce::String& s)
    {
        if (s == "High Pass") return FilterType::HIGHPASS;
        if (s == "Band Pass") return FilterType::BANDPASS;
        return FilterType::LOWPASS;
    }

    inline juce::String waveformToString(Waveform wf)
    {
        switch (wf)
        {
            case Waveform::SINE:     return "Sine";
            case Waveform::SQUARE:   return "Square";
            case Waveform::SAW:      return "Saw";
            case Waveform::TRIANGLE: return "Triangle";
        }
        jassertfalse;
        return "Sine";
    }

    inline Waveform waveformFromString(const juce::String& s)
    {
        if (s == "Square")   return Waveform::SQUARE;
        if (s == "Saw")      return Waveform::SAW;
        if (s == "Triangle") return Waveform::TRIANGLE;
        return Waveform::SINE;
    }

    // ===== SAFE GETTERS =====
    inline double getDouble(juce::DynamicObject* obj, const juce::String& key, double defaultValue)
    {
        if (obj->hasProperty(key))
            return (double)obj->getProperty(key);

        return defaultValue;
    }

    inline juce::String getString(juce::DynamicObject* obj, const juce::String& key, const juce::String& defaultValue)
    {
        if (obj->hasProperty(key))
            return obj->getProperty(key).toString();

        return defaultValue;
    }

    // ===== TO JSON =====
    inline juce::String toJson(const PatchParams& p)
    {
        juce::DynamicObject::Ptr obj = new juce::DynamicObject();

        obj->setProperty("frequency",  p.frequency);
        obj->setProperty("volume",     p.volume);
        obj->setProperty("attack",     p.attack);
        obj->setProperty("decay",      p.decay);
        obj->setProperty("sustain",    p.sustain);
        obj->setProperty("release",    p.release);
        obj->setProperty("cutoff",     p.cutoff);
        obj->setProperty("resonance",  p.resonance);
        obj->setProperty("filterType", filterToString(p.filterType));
        obj->setProperty("waveform",   waveformToString(p.waveform));
        obj->setProperty("prompt",     p.prompt);

        return juce::JSON::toString(juce::var(obj.get()), true);
    }

    // ===== FROM JSON =====
    inline std::optional<PatchParams> fromJson(const juce::String& jsonString)
    {
        juce::var json = juce::JSON::parse(jsonString);
        if (!json.isObject())
            return std::nullopt;

        auto* obj = json.getDynamicObject();

        PatchParams p;

        p.frequency  = getDouble(obj, "frequency", 440.0);
        p.volume     = getDouble(obj, "volume", 0.8);
        p.attack     = getDouble(obj, "attack", 0.1);
        p.decay      = getDouble(obj, "decay", 0.1);
        p.sustain    = getDouble(obj, "sustain", 0.8);
        p.release    = getDouble(obj, "release", 0.5);
        p.cutoff     = getDouble(obj, "cutoff", 1000.0);
        p.resonance  = getDouble(obj, "resonance", 1.0);

        p.filterType = filterFromString(
            getString(obj, "filterType", juce::String("Low Pass"))
        );

        p.waveform = waveformFromString(
            getString(obj, "waveform", juce::String("Sine"))
        );

        p.prompt = getString(obj, "prompt", juce::String());

        return p;
    }
}