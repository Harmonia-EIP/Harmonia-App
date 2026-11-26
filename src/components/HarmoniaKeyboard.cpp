#include "HarmoniaKeyboard.h"
#include "../themes/HarmoniaPalette.h"

HarmoniaKeyboard::HarmoniaKeyboard (juce::MidiKeyboardState& state)
    : juce::MidiKeyboardComponent (state, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    setScrollButtonsVisible (false);   // plus de flèches : l'affichage suit l'octave active
    applyKeyMappings();
    setKeyVelocity (defaultVelocity);
    applyColours();
}

void HarmoniaKeyboard::applyColours()
{
    // touches sombres, accordées au thème ; l'accent signale ce qu'on joue
    const auto acc = HarmoniaPalette::accent;
    setColour (whiteNoteColourId,              juce::Colour (0xFF2C3137));
    setColour (blackNoteColourId,              juce::Colour (0xFF0F1114));
    setColour (keySeparatorLineColourId,       juce::Colour (0xFF15181C));
    setColour (mouseOverKeyOverlayColourId,    acc.withAlpha (0.18f));
    setColour (keyDownOverlayColourId,         acc.withAlpha (0.85f));
    setColour (textLabelColourId,              HarmoniaPalette::textSecondary);   // "C3"... (> 4.5:1)
    setColour (shadowColourId,                 juce::Colours::transparentBlack);
    repaint();
}

void HarmoniaKeyboard::resized()
{
    // setAvailableRange() / setKeyWidth() rappellent resized() : garde anti-réentrance
    if (inResized)
    {
        juce::MidiKeyboardComponent::resized();
        return;
    }

    const juce::ScopedValueSetter<bool> guard (inResized, true);

    if (viewStart < 0)
        keepMappedNotesVisible();
    else
        applyView();   // un redimensionnement ne ramène pas l'affichage sur l'octave

    juce::MidiKeyboardComponent::resized();
}

//==============================================================================
void HarmoniaKeyboard::applyKeyMappings()
{
    // clearKeyMappings() relâche aussi les notes tenues au clavier : en changeant
    // d'octave, une touche encore enfoncée ne laisse pas l'ancienne note bloquée
    // (elle rejoue à la nouvelle octave au prochain événement clavier).
    clearKeyMappings();

    for (int i = 0; i < numMappedNotes; ++i)
        setKeyPressForNote (juce::KeyPress (currentKeys.notes[i], 0, 0), i);

    setKeyPressBaseOctave (octave);
}

void HarmoniaKeyboard::setOctave (int newOctave)
{
    newOctave = juce::jlimit (minOctave, maxOctave, newOctave);

    if (newOctave != octave)
    {
        octave = newOctave;
        applyKeyMappings();
        keepMappedNotesVisible();
        repaint();
    }

    notifySettingsChanged();
}

void HarmoniaKeyboard::setLayout (Layout newLayout)
{
    if (newLayout == layout)
        return;

    layout = newLayout;
    currentKeys = keysFor (newLayout);
    applyKeyMappings();
    repaint();

    if (onLayoutChanged)
        onLayoutChanged (layout);

    notifySettingsChanged();
}

void HarmoniaKeyboard::setKeyVelocity (float newVelocity)
{
    // arrondi au pas : 0.1 + 0.1 + ... ne dérive pas
    keyVelocity = juce::jlimit (minVelocity, 1.0f, std::round (newVelocity / velocityStep) * velocityStep);
    setVelocity (keyVelocity, false);   // false : même force au clic, quelle que soit la hauteur
    notifySettingsChanged();
}

juce::String HarmoniaKeyboard::noteName (int midiNote) const
{
    return juce::MidiMessage::getMidiNoteName (midiNote, true, true, getOctaveForMiddleC());
}

//==============================================================================
bool HarmoniaKeyboard::keyPressed (const juce::KeyPress& key)
{
    // avec Ctrl / Alt / Cmd : raccourci de l'hôte, pas pour nous
    if (key.getModifiers().isCommandDown() || key.getModifiers().isAltDown() || key.getModifiers().isCtrlDown())
        return juce::MidiKeyboardComponent::keyPressed (key);

    // flèches et Échap : pour le contrôle ciblé (voir MainComponent), pas pour le piano
    if (onArrowKey)
    {
        const int dir = (key.isKeyCode (juce::KeyPress::upKey)   || key.isKeyCode (juce::KeyPress::rightKey)) ? +1
                      : (key.isKeyCode (juce::KeyPress::downKey) || key.isKeyCode (juce::KeyPress::leftKey))  ? -1
                      : 0;

        if (dir != 0)
            return onArrowKey (dir, key.getModifiers().isShiftDown());
    }

    if (key.isKeyCode (juce::KeyPress::escapeKey) && onEscapeKey)
        return onEscapeKey();

    const auto c = juce::CharacterFunctions::toLowerCase (key.getTextCharacter() != 0
                                                              ? key.getTextCharacter()
                                                              : (juce::juce_wchar) key.getKeyCode());

    const auto& k = currentKeys;

    if (c == k.octaveDown)    { shiftOctave (-1);                           return true; }
    if (c == k.octaveUp)      { shiftOctave (+1);                           return true; }
    if (c == k.velocityDown)  { changeKeyVelocity (-velocityStep);          return true; }
    if (c == k.velocityUp)    { changeKeyVelocity (+velocityStep);          return true; }
    if (c == switchLayoutKey) { toggleLayout();                             return true; }

    return juce::MidiKeyboardComponent::keyPressed (key);
}

juce::String HarmoniaKeyboard::getTooltip()
{
    const auto& k = currentKeys;
    auto up = [] (char ch) { return juce::String::charToString ((juce::juce_wchar) juce::CharacterFunctions::toUpperCase ((juce::juce_wchar) ch)); };

    juce::StringArray notes;
    for (int i = 0; i < numMappedNotes; ++i)
        notes.add (up (k.notes[i]));

    return juce::String (k.name) + " keys: " + notes.joinIntoString (" ") + "\n"
         + up (k.octaveDown) + " / " + up (k.octaveUp) + ": octave down / up  -  "
         + up (k.velocityDown) + " / " + up (k.velocityUp) + ": velocity down / up  -  "
         + up (switchLayoutKey) + ": switch AZERTY / QWERTY";
}

//==============================================================================
void HarmoniaKeyboard::keepMappedNotesVisible()
{
    viewStart = getFirstMappedNote() - 12;   // une octave de marge sous la zone jouable (un do : blanche)
    applyView();
}

namespace
{
    int nextWhite (int note, int dir)
    {
        do { note += dir; } while (note >= 0 && note <= 127 && juce::MidiMessage::isMidiNoteBlack (note));
        return note;
    }

    /** Dernière touche d'une vue de `whiteKeys` touches blanches commençant à `lo` (blanche). */
    int viewEnd (int lo, int whiteKeys)
    {
        int hi = lo;
        for (int i = 1; i < whiteKeys; ++i)
            hi = nextWhite (hi, +1);
        return hi;
    }
}

void HarmoniaKeyboard::applyView()
{
    // Sans les flèches, JUCE remet toujours la première touche au début de la plage :
    // on ne fait donc pas défiler, on déplace la PLAGE.
    int lo = juce::jlimit (0, 127, viewStart);
    if (juce::MidiMessage::isMidiNoteBlack (lo))
        lo = nextWhite (lo, -1);

    // pas au-delà de la note 127 : on recule le début si besoin
    while (lo > 0 && viewEnd (lo, visibleWhiteKeys) > 127)
        lo = nextWhite (lo, -1);

    viewStart = lo;
    setAvailableRange (lo, juce::jmin (127, viewEnd (lo, visibleWhiteKeys)));

    if (getWidth() > 0)
        setKeyWidth ((float) getWidth() / (float) visibleWhiteKeys);
}

void HarmoniaKeyboard::slideView (int whiteKeys)
{
    int lo = viewStart < 0 ? getRangeStart() : viewStart;

    for (int i = 0; i < std::abs (whiteKeys); ++i)
    {
        const int next = nextWhite (lo, whiteKeys > 0 ? +1 : -1);
        if (next < 0 || viewEnd (next, visibleWhiteKeys) > 127)
            break;   // butée
        lo = next;
    }

    if (lo != viewStart)
    {
        viewStart = lo;
        applyView();
        repaint();
    }
}

void HarmoniaKeyboard::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    // pavé tactile : glissement horizontal ; molette : vertical (vers le bas = vers les aigus)
    float d = std::abs (wheel.deltaX) > std::abs (wheel.deltaY) ? wheel.deltaX : -wheel.deltaY;
    if (wheel.isReversed)
        d = -d;

    // changement de sens : on repart de zéro (pas de "rattrapage" du reste accumulé)
    if ((d > 0.0f) != (wheelAccum > 0.0f))
        wheelAccum = 0.0f;

    wheelAccum += d;

    const int keys = (int) (wheelAccum / wheelPerKey);
    if (keys != 0)
    {
        wheelAccum -= (float) keys * wheelPerKey;
        slideView (keys);
    }
}

//==============================================================================
void HarmoniaKeyboard::drawWhiteNote (int note, juce::Graphics& g, juce::Rectangle<float> area,
                                      bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour)
{
    juce::MidiKeyboardComponent::drawWhiteNote (note, g, area, isDown, isOver, lineColour, textColour);
    drawKeyHint (note, g, area, false);
}

void HarmoniaKeyboard::drawBlackNote (int note, juce::Graphics& g, juce::Rectangle<float> area,
                                      bool isDown, bool isOver, juce::Colour noteFillColour)
{
    juce::MidiKeyboardComponent::drawBlackNote (note, g, area, isDown, isOver, noteFillColour);
    drawKeyHint (note, g, area, true);
}

void HarmoniaKeyboard::drawKeyHint (int note, juce::Graphics& g, juce::Rectangle<float> area, bool black)
{
    const int offset = note - getFirstMappedNote();

    // hors de la zone jouable au clavier d'ordinateur : estompé (toujours jouable à la souris)
    if (! isMapped (note))
    {
        g.setColour (juce::Colours::black.withAlpha (black ? 0.30f : 0.38f));
        g.fillRect (area);
        return;
    }

    const auto acc = HarmoniaPalette::accent;

    // liseré d'accent en bas des touches jouables
    g.setColour (acc.withAlpha (black ? 0.95f : 0.85f));
    g.fillRect (area.withTop (area.getBottom() - 3.0f).reduced (1.0f, 0.0f));

    // lettre de la touche, claire sur les touches sombres (contraste > 4.5:1)
    const auto letter = juce::String::charToString (juce::CharacterFunctions::toUpperCase ((juce::juce_wchar) currentKeys.notes[offset]));
    g.setColour (juce::Colour (0xFFE0E5EA));
    g.setFont (juce::Font (juce::FontOptions (black ? 9.0f : 10.0f).withStyle ("Bold")));

    const float textH = 12.0f;
    const auto textArea = area.withTop (area.getBottom() - textH - (black ? 6.0f : 18.0f)).withHeight (textH);
    g.drawText (letter, textArea, juce::Justification::centred, false);
}
