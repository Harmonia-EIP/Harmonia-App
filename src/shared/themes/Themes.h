#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

struct ThemePalette
{
    juce::Colour background;
    juce::Colour panelBg;
    juce::Colour panelOutline;

    juce::Colour textPrimary;
    juce::Colour textSecondary;

    juce::Colour accent;
    juce::Colour accentHover;
    juce::Colour accentDown;

    juce::Colour knobBg;
    juce::Colour knobOutline;
    juce::Colour knobPointer;

    static ThemePalette dark()
    {
        ThemePalette t;
        t.background    = juce::Colour (14, 16, 18);
        t.panelBg       = juce::Colour (22, 25, 28);
        t.panelOutline  = juce::Colour (45, 48, 52);

        t.textPrimary   = juce::Colour (235, 235, 235);
        t.textSecondary = juce::Colour (160, 165, 170);

        t.accent        = juce::Colour (90, 165, 255);
        t.accentHover   = juce::Colour (120, 185, 255);
        t.accentDown    = juce::Colour (60, 135, 225);

        t.knobBg        = juce::Colour (30, 34, 38);
        t.knobOutline   = juce::Colour (0, 0, 0).withAlpha (0.6f);
        t.knobPointer   = t.accent;

        return t;
    }

    static ThemePalette light()
    {
        ThemePalette t;
        t.background    = juce::Colour (245, 245, 245);
        t.panelBg       = juce::Colour (255, 255, 255);
        t.panelOutline  = juce::Colour (210, 210, 210);

        t.textPrimary   = juce::Colour (18, 18, 18);
        t.textSecondary = juce::Colour (90, 90, 90);

        t.accent        = juce::Colour (30, 120, 220);
        t.accentHover   = juce::Colour (55, 145, 240);
        t.accentDown    = juce::Colour (20, 95, 190);

        t.knobBg        = juce::Colour (240, 240, 240);
        t.knobOutline   = juce::Colour (185, 185, 185);
        t.knobPointer   = t.accent;

        return t;
    }

    // Thème rouge complet : fonds + panels + outlines + accent rouges/rosés
    static ThemePalette red()
    {
        ThemePalette t;
        t.background    = juce::Colour (28, 10, 12);   // rouge très sombre
        t.panelBg       = juce::Colour (44, 14, 18);   // panel rouge sombre
        t.panelOutline  = juce::Colour (90, 30, 38);   // outline rouge

        t.textPrimary   = juce::Colour (245, 235, 235);
        t.textSecondary = juce::Colour (195, 165, 170);

        t.accent        = juce::Colour (235, 80, 95);  // rouge/rose vif
        t.accentHover   = juce::Colour (245, 110, 125);
        t.accentDown    = juce::Colour (200, 55, 70);

        t.knobBg        = juce::Colour (55, 18, 22);
        t.knobOutline   = juce::Colour (0, 0, 0).withAlpha (0.55f);
        t.knobPointer   = t.accent;

        return t;
    }

    // Thème bleu complet : fonds + panels + outlines + accent bleus
    static ThemePalette blue()
    {
        ThemePalette t;
        t.background    = juce::Colour (8, 14, 28);    // bleu très sombre
        t.panelBg       = juce::Colour (12, 22, 44);   // panel bleu sombre
        t.panelOutline  = juce::Colour (35, 65, 105);  // outline bleu

        t.textPrimary   = juce::Colour (235, 242, 255);
        t.textSecondary = juce::Colour (160, 180, 205);

        t.accent        = juce::Colour (80, 155, 255); // bleu vif
        t.accentHover   = juce::Colour (110, 180, 255);
        t.accentDown    = juce::Colour (55, 125, 230);

        t.knobBg        = juce::Colour (18, 30, 60);
        t.knobOutline   = juce::Colour (0, 0, 0).withAlpha (0.55f);
        t.knobPointer   = t.accent;

        return t;
    }
};
