#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

/**
 * @class HarmoniaKeyboard
 * @brief Piano du plugin, jouable au clavier d'ordinateur, en AZERTY ou en QWERTY.
 *
 *                 AZERTY                               QWERTY
 *   Notes (17)    Q Z S E D F T G Y H U J K O L P M    A W S E D F T G Y H U J K O L P ;
 *   Octave - / +  W / X                                Z / X
 *   Vélocité -/+  C / V                                C / V
 *   Disposition   B                                    B
 *
 * Les deux dispositions utilisent les MÊMES touches physiques : seules les lettres
 * changent. Changer d'octave ou de disposition relâche les notes tenues.
 *
 * Rendu : touches sombres (thème du plugin), accent quand on joue ; les 17 touches
 * jouables portent leur lettre et un liseré d'accent, les autres sont estompées.
 * Pas de flèches : 29 touches blanches remplissent la largeur. L'affichage suit l'octave
 * active (W / X, chips du panneau), et la molette ou le pavé tactile le font glisser
 * touche par touche (l'octave jouée au clavier d'ordinateur ne change pas).
 */
class HarmoniaKeyboard : public juce::MidiKeyboardComponent,
                         public juce::TooltipClient
{
public:
    enum class Layout { Azerty, Qwerty };

    /** Les touches d'une disposition (en minuscules). */
    struct LayoutKeys
    {
        const char* notes;        // une lettre par demi-ton à partir du do de l'octave active
        char octaveDown, octaveUp, velocityDown, velocityUp;
        const char* name;
    };

    static constexpr int  numMappedNotes = 17;
    static constexpr char switchLayoutKey = 'b';   // même place dans les deux dispositions

    static constexpr LayoutKeys keysFor (Layout l)
    {
        return l == Layout::Azerty
             ? LayoutKeys { "qzsedftgyhujkolpm", 'w', 'x', 'c', 'v', "AZERTY" }
             : LayoutKeys { "awsedftgyhujkolp;", 'z', 'x', 'c', 'v', "QWERTY" };
    }

    static constexpr Layout defaultLayout = Layout::Azerty;

    /** Octave 5 : la première touche (Q) joue le do central (note MIDI 60). */
    static constexpr int defaultOctave = 5;
    static constexpr int minOctave = 0;
    static constexpr int maxOctave = 9;   // 9 x 12 + 16 = 124 <= 127

    static constexpr float defaultVelocity = 0.8f;
    static constexpr float velocityStep    = 0.1f;
    static constexpr float minVelocity     = 0.1f;

    explicit HarmoniaKeyboard (juce::MidiKeyboardState& state);

    Layout getLayout() const noexcept          { return layout; }
    const LayoutKeys& keys() const noexcept    { return currentKeys; }

    /** Change de disposition (relâche les notes tenues). */
    void setLayout (Layout newLayout);
    void toggleLayout()                        { setLayout (layout == Layout::Azerty ? Layout::Qwerty : Layout::Azerty); }

    /** Appelé quand l'utilisateur change de disposition (pour la retenir). */
    std::function<void (Layout)> onLayoutChanged;

    /** Appelé à chaque changement d'octave, de vélocité ou de disposition (chips du panneau). */
    std::function<void()> onSettingsChanged;

    /** Flèches : le piano garde le focus (pour jouer) et les transmet au dernier contrôle cliqué.
        dir = +1 (haut / droite) ou -1 (bas / gauche), fine = Maj enfoncée. true = touche utilisée. */
    std::function<bool (int dir, bool fine)> onArrowKey;

    /** Échap (retire la cible des flèches). true = touche utilisée. */
    std::function<bool()> onEscapeKey;

    /** Touches blanches affichées en même temps (4 octaves + 1) : largeur des touches constante. */
    static constexpr int visibleWhiteKeys = 29;

    /** Molette : quantité de défilement pour une touche blanche (molette ~0.11 par cran, pavé en continu). */
    static constexpr float wheelPerKey = 0.04f;

    int   getOctave() const noexcept           { return octave; }
    float getKeyVelocity() const noexcept      { return keyVelocity; }

    /** Première et dernière note jouables au clavier d'ordinateur. */
    int getFirstMappedNote() const noexcept    { return 12 * octave; }
    int getLastMappedNote() const noexcept     { return 12 * octave + numMappedNotes - 1; }

    void setOctave (int newOctave);
    void shiftOctave (int delta)               { setOctave (octave + delta); }

    void setKeyVelocity (float newVelocity);
    void changeKeyVelocity (float delta)       { setKeyVelocity (keyVelocity + delta); }

    /** "C3", "C#4"... avec la même convention que les étiquettes du piano. */
    juce::String noteName (int midiNote) const;

    bool keyPressed (const juce::KeyPress& key) override;
    juce::String getTooltip() override;

    void resized() override;
    void lookAndFeelChanged() override   { applyColours(); }

    /** Cadre l'affichage sur l'octave active (une octave de marge à gauche). */
    void keepMappedNotesVisible();

    /** Fait glisser l'affichage de `whiteKeys` touches blanches (négatif = vers les graves). */
    void slideView (int whiteKeys);

    void mouseWheelMove (const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;

    /** Plage de notes affichée (pour les tests). */
    juce::Range<int> getVisibleRange() const noexcept { return { getRangeStart(), getRangeEnd() }; }

protected:
    void drawWhiteNote (int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                        bool isDown, bool isOver, juce::Colour lineColour, juce::Colour textColour) override;

    void drawBlackNote (int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area,
                        bool isDown, bool isOver, juce::Colour noteFillColour) override;

private:
    void applyKeyMappings();
    void applyColours();
    void notifySettingsChanged()               { if (onSettingsChanged) onSettingsChanged(); }
    bool isMapped (int note) const noexcept    { return note >= getFirstMappedNote() && note <= getLastMappedNote(); }
    void drawKeyHint (int midiNoteNumber, juce::Graphics& g, juce::Rectangle<float> area, bool black);

    Layout     layout      = defaultLayout;
    LayoutKeys currentKeys = keysFor (defaultLayout);

    int   octave      = defaultOctave;
    bool  inResized   = false;
    int   viewStart   = -1;      // première touche affichée (blanche) ; -1 = pas encore placée
    float wheelAccum  = 0.0f;

    void applyView();
    float keyVelocity = defaultVelocity;


    JUCE_DECLARE_WEAK_REFERENCEABLE (HarmoniaKeyboard)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmoniaKeyboard)
};
