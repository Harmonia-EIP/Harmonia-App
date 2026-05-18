# Harmonia App – JUCE Synthesizer Frontend

Epitech Innovation Project (EIP)

Harmonia App is a modern audio synthesizer application developed in **C++17 using JUCE 8**.  
It provides both a **standalone desktop application** and a **VST3 plugin**, featuring a custom synthesizer engine, real-time DSP, waveform visualization, preset management, and backend-powered AI preset generation.

---

# 🔗 Links

- GitHub Repository: https://github.com/Harmonia-EIP/Harmonia-App
- Documentation: https://harmonia-eip.github.io/Harmonia-App/

---

# Features

## Audio Engine

- Polyphonic synthesizer engine
- Multiple oscillator waveforms
- Oscillator mixing and detune
- Noise generator
- ADSR envelope
- LFO modulation
- Multi-mode filter
- Reverb effect
- Distortion control
- MIDI keyboard support
- Real-time DSP processing

---

## Visual Components

- Oscilloscope waveform display
- Envelope visualizer
- LFO visualizer
- Animated particle system
- Modern custom JUCE LookAndFeel
- Interactive knobs and waveform selectors

---

## Backend Features

- Authentication system
- Login / Signup pages
- Session persistence
- AI-generated presets
- Preset save/load system
- HTTP backend integration using CPR

---

# Technologies Used

- C++17
- JUCE 8
- CMake
- Visual Studio 2022
- CPR (HTTP Client)
- nlohmann/json

---

# Project Structure

```text
src/
├── backendManagement/
├── components/
├── config/
├── pages/
├── parameters/
├── themes/
├── tools/
```

---

# Application Architecture

```text
PluginProcessor
        │
        ▼
PluginEditor
        │
        ▼
AppController
        │
        ▼
MainComponent
        │
 ┌──────┼────────────────────────────┐
 ▼      ▼            ▼               ▼
DSP   UI Panels   Oscilloscope   Backend/API
```

---

# Main Components

## HarmoniaAudioProcessor

Core DSP engine responsible for:

- Audio rendering
- MIDI processing
- Synth voice management
- Reverb processing
- Parameter management (APVTS)
- Oscilloscope audio streaming

---

## MainComponent

Main synthesizer interface containing:

- Oscillator controls
- Filter section
- Envelope section
- LFO section
- FX section
- Oscilloscope display
- AI prompt system
- Preset management

---

## OscilloscopeComponent

Real-time waveform renderer:

- Circular ring buffer
- Timer-driven repaint
- Stereo → mono visualization
- Glow waveform rendering

---

## HarmoniaParameters

Centralized parameter system using:

- `juce::AudioProcessorValueTreeState`
- Atomic parameter references
- Logarithmic ranges
- Shared parameter IDs

---

# Build System

The project uses:

- CMake
- JUCE as a submodule
- CPR as HTTP library
- nlohmann/json

---

# Requirements

## Windows

- Windows 10/11
- Visual Studio 2022
- CMake ≥ 3.15
- Git

---

## macOS

- Xcode
- CMake ≥ 3.15
- Apple Silicon + Intel supported (Universal2 build)

---

# Clone Repository

```bash
git clone --recurse-submodules https://github.com/Harmonia-EIP/Harmonia-App.git
cd Harmonia-App
```

If already cloned:

```bash
git submodule update --init --recursive
```

---

# Build Instructions

## Configure

```bash
cmake -S . -B build
```

## Build Release

```bash
cmake --build build --config Release
```

---

# Generated Outputs

## Standalone Application

```text
build/HarmoniaPlugin_artefacts/Release/Standalone/HarmoniaPlugin.exe
```

---

## VST3 Plugin

```text
build/HarmoniaPlugin_artefacts/Release/VST3/HarmoniaPlugin.vst3
```

Copy the `.vst3` file into your DAW VST3 folder.

---

# Running the Application

## Standalone Version

Launch:

```bash
build/Release/HarmoniaPlugin.exe
```

---

## VST3 Version

Load the plugin inside a DAW such as:

- Ableton Live
- FL Studio
- Reaper
- Cubase

---

# CMake Configuration

Main dependencies configured in CMake:

- JUCE
- CPR
- nlohmann/json

The project supports:

- Standalone app
- VST3 plugin
- Universal macOS binaries
- Static dependency linking

---

# Audio Features

## Oscillators

Available waveforms:

- Sine
- Triangle
- Saw
- Square

---

## Modulation

- LFO → Pitch
- LFO → Filter cutoff
- Velocity → Filter

---

## Effects

- Reverb
- Distortion

---

# Preset System

Supports:

- Save presets as JSON
- Load presets from JSON
- AI-generated presets
- User metadata

---

# Backend Integration

The frontend communicates with backend services for:

- Authentication
- Session management
- AI preset generation
- Cloud synchronization

Communication is handled through CPR HTTP requests.

---

# Notes

- `build/` is auto-generated
- JUCE is included as a Git submodule
- Do not modify JUCE source directly
- Recommended build mode: `Release`

---

# Team

Developed as part of the Epitech Innovation Project (EIP)

---

# License

Academic project  
License to be defined as the project evolves.