#include "PromptEditor.h"

void PromptEditor::setHint (const juce::String& text, juce::Colour colour, bool monospace)
{
    hint = text;
    hintCol = colour;
    mono = monospace;
    repaint();
}

void PromptEditor::paint (juce::Graphics& g)
{
    juce::TextEditor::paint (g);

    if (getTotalNumChars() > 0 || hasKeyboardFocus (false))
        return;

    // même taille visuelle dans les deux modes (voir PromptFont)
    const juce::Font font = mono
        ? PromptFont::make (true)
        : juce::Font (juce::FontOptions (PromptFont::size).withStyle ("Italic")).withExtraKerningFactor (0.02f);

    g.setColour (hintCol);
    g.setFont (font);
    g.drawText (hint, getLocalBounds().withTrimmedLeft (4),
                juce::Justification::centredLeft, true);
}

void PromptEditor::focusGained (FocusChangeType t)
{
    juce::TextEditor::focusGained (t);
    if (onFocusChanged) onFocusChanged();
}

void PromptEditor::focusLost (FocusChangeType t)
{
    juce::TextEditor::focusLost (t);
    if (onFocusChanged) onFocusChanged();
}
