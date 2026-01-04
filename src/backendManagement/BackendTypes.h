#pragma once

#include <juce_core/juce_core.h>

struct UserSession
{
    juce::String pseudo;
    juce::String email;
    juce::String accessToken;
    juce::Time   expiresAt;
};

struct AuthResult
{
    bool success = false;
    UserSession session;
    juce::String errorMessage;
};

struct PatchParams
{
    juce::String waveform;
    juce::String filterType;

    double frequency  = 440.0;
    double volume     = 0.8;
    double attack     = 0.1;
    double decay      = 0.1;
    double sustain    = 0.8;
    double release    = 0.5;
    double cutoff     = 1000.0;
    double resonance  = 1.0;
};

struct PatchCallResult
{
    bool success = false;
    PatchParams params;
    juce::String errorMessage;
};
