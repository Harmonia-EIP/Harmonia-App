/**
 * @file ComboControl.h
 * @brief UI component used to display and control a choice parameter.
 *
 * ComboControl provides a styled wrapper around a JUCE ComboBox
 * connected to an AudioProcessorValueTreeState choice parameter.
 *
 * Features:
 * - Automatic parameter attachment
 * - Dynamic choice population
 * - Centered caption rendering
 * - Integrated Harmonia styling
 */
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"

/**
 * @class ComboControl
 * @brief Dropdown selector component linked to an APVTS choice parameter.
 *
 * This component automatically retrieves available choices
 * from a juce::AudioParameterChoice and synchronizes the UI
 * with the underlying parameter through a ComboBoxAttachment.
 */
class ComboControl : public juce::Component
{
public:
    /**
     * @brief Creates a ComboControl linked to an APVTS parameter.
     *
     * @param apvts Reference to the AudioProcessorValueTreeState.
     * @param paramId Parameter identifier.
     * @param displayName Text displayed above the combo box.
     */
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

    /**
     * @brief Paints the component caption.
     *
     * @param g Graphics context.
     */
    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds();
        g.setColour (HarmoniaPalette::textMuted);
        g.setFont (juce::Font (juce::FontOptions (9.5f).withStyle ("Bold")));
        g.drawText (caption.toUpperCase(),
                    r.withHeight (captionH),
                    juce::Justification::centred);
    }

    /**
     * @brief Updates component layout.
     */
    void resized() override
    {
        auto r = getLocalBounds();
        r.removeFromTop (captionH);
        combo.setBounds (r.reduced (4, 2));
    }

private:
    /**
     * @brief ComboBox used for parameter selection.
     */
    juce::ComboBox combo;

    /**
     * @brief Displayed component caption.
     */
    juce::String caption;

    /**
     * @brief APVTS attachment synchronizing parameter and ComboBox.
     */
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> attachment;

    /**
     * @brief Caption height in pixels.
     */
    static constexpr int captionH = 14;
};
