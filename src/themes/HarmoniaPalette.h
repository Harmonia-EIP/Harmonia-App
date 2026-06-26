#pragma once
#include <juce_graphics/juce_graphics.h>

namespace HarmoniaPalette
{
    enum class Theme
    { 
        Cyan = 0,
        Blue,
        Red,
        Purple,
        Dark
    };

    static HarmoniaPalette::Theme themeFromId (int id)
    {
        using T = HarmoniaPalette::Theme;

        switch (id)
        {
            case 0: return T::Cyan;
            case 1: return T::Blue;
            case 2: return T::Red;
            case 3: return T::Purple;
            case 4: return T::Dark;

            default:
                return T::Cyan;
        }
    }

    // ── dynamic accent colours ────────────────────────────────────────────
    inline juce::Colour accent        { 0xFF00D4FF };
    inline juce::Colour accentSecond  { 0xFF00B5DB };
    inline juce::Colour accentHover   { 0xFF4FE2FF };

    // ── dynamic section colours ───────────────────────────────────────────
    inline juce::Colour sectionOsc1 { 0xFFE38202 };
    inline juce::Colour sectionOsc2 { 0xFF3852FC };

    inline juce::Colour sectionFilter { 0xFFA78BFA };
    inline juce::Colour sectionDisplay{ 0xFF00D4FF };
    inline juce::Colour sectionLfo    { 0xFF34D399 };
    inline juce::Colour sectionFx     { 0xFFEB6F92 };
    inline juce::Colour sectionAmpEnv { 0xFFFBBF24 };

    // ── dynamic text colours ──────────────────────────────────────────────
    inline juce::Colour textPrimary   { 0xFFE0E5EA };
    inline juce::Colour textSecondary { 0xFFA0A5AB };
    inline juce::Colour textMuted     { 0xFF6B7178 };

    // ── fixed surfaces (DO NOT CHANGE WITH THEMES) ───────────────────────
    inline const juce::Colour background  { 0xFF1E2226 };
    inline const juce::Colour panel       { 0xFF262A30 };
    inline const juce::Colour panelHi     { 0xFF2C3137 };
    inline const juce::Colour panelTop    { 0xFF2A2F35 };
    inline const juce::Colour border      { 0xFF15181C };
    inline const juce::Colour borderHi    { 0xFF2A2F35 };

    inline const juce::Colour warm        { 0xFFEB6F92 };

    inline const juce::Colour knobBg      { 0xFF1A1E22 };
    inline const juce::Colour knobBgHi    { 0xFF22262A };
    inline const juce::Colour knobTrack   { 0xFF15181C };
    inline const juce::Colour screenBg    { 0xFF0A0D11 };
    inline const juce::Colour screenGrid  { 0xFF1F2730 };

    // ──────────────────────────────────────────────────────────────────────

    struct ThemeDef
    {
        juce::Colour accent, accentSecond, accentHover;

        juce::Colour osc;
        juce::Colour filter;
        juce::Colour display;
        juce::Colour lfo;
        juce::Colour fx;
        juce::Colour ampEnv;

        juce::Colour text1;
        juce::Colour text2;
        juce::Colour textMutedCol;
    };

    inline ThemeDef makeTheme (
        juce::uint32 a,
        juce::uint32 aS,
        juce::uint32 aH,

        juce::uint32 osc_,
        juce::uint32 filt,
        juce::uint32 disp,
        juce::uint32 lfo_,
        juce::uint32 fx_,
        juce::uint32 amp,

        juce::uint32 t1,
        juce::uint32 t2,
        juce::uint32 tm)
    {
        return {
            juce::Colour (a),
            juce::Colour (aS),
            juce::Colour (aH),

            juce::Colour (osc_),
            juce::Colour (filt),
            juce::Colour (disp),
            juce::Colour (lfo_),
            juce::Colour (fx_),
            juce::Colour (amp),

            juce::Colour (t1),
            juce::Colour (t2),
            juce::Colour (tm)
        };
    }

    // ── THEMES ────────────────────────────────────────────────────────────

    inline const ThemeDef Themes[] =
    {
        // CYAN
        makeTheme (
            0xFF00D4FF,
            0xFF00B5DB,
            0xFF4FE2FF,

            0xFF00D4FF,
            0xFFA78BFA,
            0xFF00D4FF,
            0xFF34D399,
            0xFFEB6F92,
            0xFFFBBF24,

            0xFFE0F9FF,
            0xFF8FE8FF,
            0xFF4F7D88),

        // BLUE
        makeTheme (
            0xFF2563FF,
            0xFF163FD1,
            0xFF7DA2FF,

            0xFF2563FF,
            0xFF7B8DFF,
            0xFF2563FF,
            0xFF6DFC95,
            0xFFEB6F92,
            0xFFFBBF24,

            0xFFAED0FF,
            0xFF7DA2FF,
            0xFF4F6C99),

        // RED
        makeTheme (
            0xFFFF3B5C,
            0xFFD91F45,
            0xFFFF7D95,

            0xFFFF3B5C,
            0xFFFF6B81,
            0xFFFF3B5C,
            0xFFFF9A76,
            0xFFFF5DA2,
            0xFFFFC857,

            0xFFFFD0D8,
            0xFFFF8AA0,
            0xFF8A4A58),

        // PURPLE
        makeTheme (
            0xFF7B2CFF,
            0xFF5818CC,
            0xFFC08BFF,

            0xFF7B2CFF,
            0xFFFF5DB1,
            0xFF7B2CFF,
            0xFF8BFFB0,
            0xFFFF6FCF,
            0xFFFBBF24,

            0xFFE0CCFF,
            0xFFC08BFF,
            0xFF6B4E99),

        // DARK
        makeTheme (
            0xFFB0B8C8,
            0xFF8892A4,
            0xFFD0D8E8,

            0xFFB0B8C8,
            0xFFA78BFA,
            0xFFB0B8C8,
            0xFF34D399,
            0xFFEB6F92,
            0xFFFBBF24,

            0xFFE0E5EA,
            0xFFA0A5AB,
            0xFF6B7178)
    };

    // ── APPLY THEME ───────────────────────────────────────────────────────

    inline void setTheme (Theme t)
    {
        const auto& d = Themes[static_cast<int>(t)];

        accent         = d.accent;
        accentSecond   = d.accentSecond;
        accentHover    = d.accentHover;

        sectionFilter  = d.filter;
        sectionDisplay = d.display;
        sectionLfo     = d.lfo;
        sectionFx      = d.fx;
        sectionAmpEnv  = d.ampEnv;

        textPrimary    = d.text1;
        textSecondary  = d.text2;
        textMuted      = d.textMutedCol;

        sectionOsc1    = d.osc;
        sectionOsc2    = d.osc;
    }

    inline juce::Colour themePreviewColour (Theme t)
    {
        return Themes[static_cast<int>(t)].accent;
    }
};