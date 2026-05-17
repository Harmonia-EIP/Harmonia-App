#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"

class ComboControl : public juce::Component
{
public:
    ComboControl (juce::AudioProcessorValueTreeState& apvts,
                  const juce::String& paramId,
                  const juce::String& displayName)
        : caption (displayName)
    {
        if (auto* param = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (paramId)))
        {
            int id = 1;
            for (const auto& choice : param->choices)
                combo.addItem (choice, id++);
        }
        combo.setJustificationType (juce::Justification::centred);
        addAndMakeVisible (combo);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
            apvts, paramId, combo);
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds();
        g.setColour (HarmoniaPalette::textMuted);
        g.setFont (juce::Font (juce::FontOptions (9.5f).withStyle ("Bold")));
        g.drawText (caption.toUpperCase(),
                    r.withHeight (captionH),
                    juce::Justification::centred);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        r.removeFromTop (captionH);
        combo.setBounds (r.reduced (4, 2));
    }

private:
    juce::ComboBox combo;
    juce::String caption;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;
    static constexpr int captionH = 14;
};
