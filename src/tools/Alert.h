#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include <map>
#include <vector>

/**
 * Toast Harmonia : carte sombre avec lueur d'accent, affichée quelques secondes
 * puis disparue toute seule. Header-only.
 *
 *   HarmoniaAlert::error (this, "Authentication error", message);
 *   HarmoniaAlert::error (this, "Authentication error", message, HarmoniaAlert::Position::Center);
 *   HarmoniaAlert::setDefaultPosition (HarmoniaAlert::Position::TopRight);
 *
 * - Survol souris : le compte à rebours est mis en pause.
 * - Clic : ferme tout de suite.
 * - Même message déjà affiché : le timer est simplement relancé (pas de doublon).
 */
class HarmoniaAlert : public juce::Component,
                      private juce::Timer,
                      private juce::ComponentListener
{
public:
    enum class Type { Info, Success, Warning, Error };

    enum class Position
    {
        Default,            // = position par défaut (setDefaultPosition)
        TopLeft, TopCenter, TopRight,
        Center,
        BottomLeft, BottomCenter, BottomRight
    };

    // ------------------------------------------------------------------
    // API
    // ------------------------------------------------------------------
    static void info    (juce::Component* parent, const juce::String& title, const juce::String& message,
                         Position pos = Position::Default, int durationMs = 4000)
    { show (parent, Type::Info, title, message, pos, durationMs); }

    static void success (juce::Component* parent, const juce::String& title, const juce::String& message,
                         Position pos = Position::Default, int durationMs = 3500)
    { show (parent, Type::Success, title, message, pos, durationMs); }

    static void warning (juce::Component* parent, const juce::String& title, const juce::String& message,
                         Position pos = Position::Default, int durationMs = 4500)
    { show (parent, Type::Warning, title, message, pos, durationMs); }

    static void error   (juce::Component* parent, const juce::String& title, const juce::String& message,
                         Position pos = Position::Default, int durationMs = 5500)
    { show (parent, Type::Error, title, message, pos, durationMs); }

    static void setDefaultPosition (Position p)
    {
        if (p != Position::Default)
            defaultPositionRef() = p;
    }

    static void show (juce::Component* parent, Type type,
                      const juce::String& title, const juce::String& message,
                      Position pos = Position::Default, int durationMs = 4000)
    {
        if (parent == nullptr)
            return;

        auto* root = parent->getTopLevelComponent();
        if (root == nullptr)
            root = parent;

        if (pos == Position::Default)
            pos = defaultPositionRef();

        // Même toast déjà visible : on relance son timer au lieu d'en empiler un second
        for (auto* t : active())
        {
            if (t->root.getComponent() == root && t->position == pos
                && t->type == type && t->title == title && t->message == message)
            {
                t->elapsed = 0.0;
                t->closing = false;
                return;
            }
        }

        new HarmoniaAlert (*root, type, title, message, pos, durationMs);   // se supprime seul
    }

    ~HarmoniaAlert() override
    {
        stopTimer();

        auto& list = active();
        list.erase (std::remove (list.begin(), list.end(), this), list.end());

        if (root != nullptr)
            root->removeComponentListener (this);

        layoutAll();   // les autres toasts remontent
    }

    // ------------------------------------------------------------------
    // Rendu
    // ------------------------------------------------------------------
    void paint (juce::Graphics& g) override
    {
        const float ease   = 1.f - std::pow (1.f - appear, 3.f);
        const auto  accent = accentFor (type);
        const auto  card   = getLocalBounds().toFloat().reduced ((float) glowMargin);

        g.beginTransparencyLayer (ease);

        {
            juce::Graphics::ScopedSaveState ss (g);

            // petit glissement d'entrée, depuis le haut ou le bas selon la position
            g.addTransform (juce::AffineTransform::translation (0.f, (1.f - ease) * (isBottom (position) ? 8.f : -8.f)));

            // lueur (pré-calculée)
            g.drawImageAt (glowImage, 0, 0);

            // fond
            g.setGradientFill (juce::ColourGradient (bgTop(),    card.getX(), card.getY(),
                                                     bgBottom(), card.getX(), card.getBottom(), false));
            g.fillRoundedRectangle (card, cornerRadius);

            // bordure d'accent
            g.setColour (accent.withAlpha (0.55f));
            g.drawRoundedRectangle (card.reduced (0.5f), cornerRadius, 1.2f);

            // icône
            const float iconD = 34.f;
            drawIcon (g, type, juce::Rectangle<float> (iconD, iconD)
                                   .withCentre ({ card.getX() + padding + iconD * 0.5f, card.getCentreY() }));

            // titre
            const float textX = card.getX() + textOffset;
            g.setColour (textMain());
            g.setFont (juce::Font ("Space Grotesk", 14.5f, juce::Font::bold));
            g.drawText (title, juce::Rectangle<float> (textX, card.getY() + padding, (float) textWidth, titleHeight),
                        juce::Justification::centredLeft, true);

            // message
            if (message.isNotEmpty())
                messageLayout.draw (g, { textX, card.getY() + padding + titleHeight + 3.f,
                                         (float) textWidth, messageLayout.getHeight() });

            // barre de temps restant, clippée aux coins arrondis
            {
                juce::Path clip;
                clip.addRoundedRectangle (card, cornerRadius);
                g.reduceClipRegion (clip);

                const float remaining = juce::jlimit (0.f, 1.f, 1.f - (float) (elapsed / (double) durationMs));
                auto bar = card.withTop (card.getBottom() - 2.5f).withWidth (card.getWidth() * remaining);

                g.setGradientFill (juce::ColourGradient (accent.withAlpha (0.95f), bar.getX(), 0.f,
                                                         accent.withAlpha (0.35f), card.getRight(), 0.f, false));
                g.fillRect (bar);
            }
        }

        g.endTransparencyLayer();
    }

    void mouseUp (const juce::MouseEvent&) override { closing = true; }

private:
    // ------------------------------------------------------------------
    // Palette (remplace par tes HarmoniaColours::... si tu veux)
    // ------------------------------------------------------------------
    static juce::Colour bgTop()    { return juce::Colour (0xff18222f); }
    static juce::Colour bgBottom() { return juce::Colour (0xff0f1923); }
    static juce::Colour textMain() { return juce::Colour (0xfff2f5f9); }

    static juce::Colour accentFor (Type t)
    {
        switch (t)
        {
            case Type::Error:   return juce::Colour (0xffff5c6c);
            case Type::Warning: return juce::Colour (0xffffb84d);
            case Type::Success: return juce::Colour (0xff4ade80);
            case Type::Info:    break;
        }
        return juce::Colour (0xff4cc9f0);   // cyan des vagues
    }

    // ------------------------------------------------------------------
    // Dimensions
    // ------------------------------------------------------------------
    static constexpr int   cardWidth    = 340;
    static constexpr int   glowMargin   = 14;      // place autour de la carte pour la lueur
    static constexpr int   edgeGap      = 16;
    static constexpr int   topInset     = 98;      // distance carte <-> bord de la fenêtre
    static constexpr int   stackGap     = 8;
    static constexpr float cornerRadius = 14.f;
    static constexpr float padding      = 14.f;
    static constexpr float titleHeight  = 20.f;
    static constexpr int   textOffset   = 14 + 34 + 12;                    // padding + icône + espace
    static constexpr int   textWidth    = cardWidth - textOffset - 14;

    static bool isTop    (Position p) { return p == Position::TopLeft || p == Position::TopCenter || p == Position::TopRight; }
    static bool isBottom (Position p) { return p == Position::BottomLeft || p == Position::BottomCenter || p == Position::BottomRight; }
    static bool isLeft   (Position p) { return p == Position::TopLeft || p == Position::BottomLeft; }
    static bool isRight  (Position p) { return p == Position::TopRight || p == Position::BottomRight; }

    static Position& defaultPositionRef() { static Position p = Position::TopRight; return p; }
    static std::vector<HarmoniaAlert*>& active() { static std::vector<HarmoniaAlert*> v; return v; }

    // ------------------------------------------------------------------
    // Construction
    // ------------------------------------------------------------------
    HarmoniaAlert (juce::Component& rootComp, Type t, const juce::String& ti,
                   const juce::String& m, Position p, int dur)
        : root (&rootComp), type (t), title (ti), message (m), position (p),
          durationMs (juce::jmax (1000, dur))
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
        setWantsKeyboardFocus (false);

        buildLayoutAndCache();

        rootComp.addAndMakeVisible (this);
        rootComp.addComponentListener (this);
        toFront (false);

        active().push_back (this);
        layoutAll();

        lastTick = juce::Time::getMillisecondCounterHiRes();
        startTimerHz (60);
    }

    void buildLayoutAndCache()
    {
        float messageH = 0.f;

        if (message.isNotEmpty())
        {
            juce::AttributedString s;
            s.setJustification (juce::Justification::topLeft);
            s.append (message, juce::Font ("Inter", 12.f, juce::Font::plain), textMain().withAlpha (0.72f));
            messageLayout.createLayout (s, (float) textWidth);
            messageH = messageLayout.getHeight() + 3.f;
        }

        const float cardH = juce::jmax (padding * 2.f + 34.f,
                                        padding * 2.f + titleHeight + messageH + 2.f);

        setSize (cardWidth + glowMargin * 2, (int) std::ceil (cardH) + glowMargin * 2);

        // lueur pré-calculée une fois (le flou à 60 Hz serait inutilement coûteux)
        glowImage = juce::Image (juce::Image::ARGB, getWidth(), getHeight(), true);
        juce::Graphics gg (glowImage);

        juce::Path shape;
        shape.addRoundedRectangle (getLocalBounds().toFloat().reduced ((float) glowMargin), cornerRadius);

        juce::DropShadow (juce::Colours::black.withAlpha (0.55f), 14, { 0, 4 }).drawForPath (gg, shape);
        juce::DropShadow (accentFor (type).withAlpha (0.28f),     12, { 0, 0 }).drawForPath (gg, shape);
    }

    // ------------------------------------------------------------------
    // Positionnement et empilement
    // ------------------------------------------------------------------
    static void layoutAll()
    {
        std::map<std::pair<juce::Component*, int>, int> used;   // décalage cumulé par (racine, position)

        for (auto* t : active())
        {
            auto* r = t->root.getComponent();

            if (r == nullptr)
                continue;

            const auto area = r->getLocalBounds();
            const int  w = t->getWidth(), h = t->getHeight();
            const int  cardH = h - glowMargin * 2;
            int& offset = used[{ r, (int) t->position }];

            int x;
            if      (isLeft  (t->position)) x = edgeGap - glowMargin;
            else if (isRight (t->position)) x = area.getRight() - edgeGap + glowMargin - w;
            else                            x = area.getCentreX() - w / 2;

            int y;
            if      (isTop    (t->position)) y = topInset - glowMargin + offset;
            else if (isBottom (t->position)) y = area.getBottom() - edgeGap + glowMargin - h - offset;
            else                              y = area.getCentreY() - h / 2 + offset;

            t->setTopLeftPosition (x, y);
            offset += cardH + stackGap;
        }
    }

    void componentMovedOrResized (juce::Component&, bool, bool) override { layoutAll(); }

    // ------------------------------------------------------------------
    // Animation
    // ------------------------------------------------------------------
    void timerCallback() override
    {
        // la fenêtre racine a disparu avant nous
        if (root == nullptr)
        {
            finish();
            return;
        }

        const double now = juce::Time::getMillisecondCounterHiRes();
        const double dt  = juce::jlimit (0.0, 100.0, now - lastTick);
        lastTick = now;

        if (closing)
        {
            appear -= (float) (dt / 300.0);

            if (appear <= 0.f)
            {
                appear = 0.f;
                finish();
                return;
            }
        }
        else
        {
            appear = juce::jmin (1.f, appear + (float) (dt / 220.0));

            if (! isMouseOver (true))      // survol = pause
                elapsed += dt;

            if (elapsed >= (double) durationMs)
                closing = true;
        }

        repaint();
    }

    void finish()
    {
        stopTimer();
        setVisible (false);

        juce::Component::SafePointer<HarmoniaAlert> self (this);
        juce::MessageManager::callAsync ([self] { delete self.getComponent(); });
    }

    // ------------------------------------------------------------------
    // Icônes dessinées (pas besoin de fichiers)
    // ------------------------------------------------------------------
    static void drawIcon (juce::Graphics& g, Type t, juce::Rectangle<float> c)
    {
        const auto col = accentFor (t);

        g.setColour (col.withAlpha (0.16f));
        g.fillEllipse (c);
        g.setColour (col.withAlpha (0.85f));
        g.drawEllipse (c.reduced (1.f), 1.4f);

        g.setColour (col);
        const auto m = c.getCentre();
        const juce::PathStrokeType stroke (2.3f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded);
        juce::Path p;

        switch (t)
        {
            case Type::Error:
                p.startNewSubPath (m.translated (-5.5f, -5.5f));
                p.lineTo          (m.translated ( 5.5f,  5.5f));
                p.startNewSubPath (m.translated ( 5.5f, -5.5f));
                p.lineTo          (m.translated (-5.5f,  5.5f));
                g.strokePath (p, stroke);
                break;

            case Type::Warning:   // "!"
                p.startNewSubPath (m.translated (0.f, -7.f));
                p.lineTo          (m.translated (0.f,  2.f));
                g.strokePath (p, stroke);
                g.fillEllipse (juce::Rectangle<float> (3.8f, 3.8f).withCentre (m.translated (0.f, 7.f)));
                break;

            case Type::Success:   // "✓"
                p.startNewSubPath (m.translated (-6.f,  0.5f));
                p.lineTo          (m.translated (-1.8f, 4.8f));
                p.lineTo          (m.translated ( 6.5f, -4.8f));
                g.strokePath (p, stroke);
                break;

            case Type::Info:      // "i"
                p.startNewSubPath (m.translated (0.f, -1.5f));
                p.lineTo          (m.translated (0.f,  7.f));
                g.strokePath (p, stroke);
                g.fillEllipse (juce::Rectangle<float> (3.8f, 3.8f).withCentre (m.translated (0.f, -7.f)));
                break;
        }
    }

    // ------------------------------------------------------------------
    // Données
    // ------------------------------------------------------------------
    juce::Component::SafePointer<juce::Component> root;
    Type type;
    juce::String title, message;
    Position position;
    int durationMs;

    juce::TextLayout messageLayout;
    juce::Image glowImage;

    float  appear   = 0.f;      // 0 -> 1 à l'entrée, 1 -> 0 à la sortie
    double elapsed  = 0.0;      // temps visible (en ms, hors pause)
    double lastTick = 0.0;
    bool   closing  = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmoniaAlert)
};