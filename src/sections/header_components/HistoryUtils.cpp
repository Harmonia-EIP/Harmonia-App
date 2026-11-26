#include "HistoryUtils.h"

namespace HistoryUtils
{
    bool isRefined (const juce::String& type)
    {
        return type.containsIgnoreCase ("refine");
    }

    juce::String formatDate (const juce::String& iso)
    {
        if (iso.length() < 19)
            return {};

        const int y   = iso.substring (0, 4).getIntValue();
        const int mo  = iso.substring (5, 7).getIntValue();
        const int d   = iso.substring (8, 10).getIntValue();
        const int h   = iso.substring (11, 13).getIntValue();
        const int mi  = iso.substring (14, 16).getIntValue();
        const int sec = iso.substring (17, 19).getIntValue();

        if (y < 2000 || mo < 1 || mo > 12 || d < 1 || d > 31)
            return {};

        const juce::Time utc (y, mo - 1, d, h, mi, sec, 0, false);   // false = interprété en UTC
        return utc.formatted ("%d %b %H:%M");                        // affiché en heure locale
    }

    juce::String formatModel (const juce::String& modelName, const juce::String& modelId,
                              const AIModelList& models)
    {
        const auto raw = (modelName.trim().isNotEmpty() ? modelName : modelId).trim();

        if (raw.isEmpty())
            return "Unknown model";

        if (raw.equalsIgnoreCase ("latest"))
            return "Latest";

        for (const auto& m : models)
            if (m.id.equalsIgnoreCase (raw) && m.name.isNotEmpty())
                return m.name;

        // "harmonia_v3" -> "Harmonia V3" : séparateurs en espaces, 1re lettre de chaque mot en majuscule
        juce::StringArray words;
        words.addTokens (raw.replaceCharacters ("_-", "  "), " ", "");
        words.removeEmptyStrings();

        for (auto& w : words)
            w = w.substring (0, 1).toUpperCase() + w.substring (1);

        return words.joinIntoString (" ");
    }
}
