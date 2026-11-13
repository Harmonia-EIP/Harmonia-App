CHANGELOG.md 📜 - 12/11/2026
All notable changes to the Harmonia plugin (JUCE / C++ interface) will be documented in this file.

[1.4.0] - Generation & Refinement History
Added
History of generations and refinements, loadable from the UI (PR #52, branch `prompt_historic`).
Custom avatar replaces the logo in the top bar once the user is signed in.
Fixed
Pseudo display no longer resizes the other components of the top bar.

[1.3.0] - AI Models Discovery and Backend Error Handling
Added
Dynamic list of AI models: the plugin now fetches the available models from the backend instead of relying on a hardcoded list (branch `check_models`).
Backend error code mapping: each error code returned by the backend is translated to a clean user-facing message (branch `exception_handler`).
New alert style used to display those errors in the UI.
Fixed
AI models check broken after the merge of `check_models`.

[1.2.0] - Backend Refactoring, Tests and Deployment Fixes
Added
Unit tests on the processor, the preset loader and the parameters.
UI tests.
Build of every target in a single step for deployment.
Changed
Backend logic now lives in the processor: the HTTP client, API errors, storage and services are split into dedicated classes (branch `top-bar-refacto`, then `refacto-backend-processor`).
Fixed
Backend URL.
Deployment workflow.

[1.1.0] - Refine Mode, Logo and Faster CI
Added
Switch between Generate and Refine mode from the top bar; the top bar options are never locked.
Lock on known parameters, only active when Refine mode is on; lock colors are updated with the backend.
Hints (tooltips) on buttons.
Refine mode is implemented but disabled for now.
New logo and reworked sign up / sign in pages.
sccache in the CI to build roughly twice as fast.
Generation and storage of the executable on the VPS.
Changed
Removed the redundant "plugin" from the `harmoniaPlugin` names.
Removed the clang-format job that auto-pushed commits and created conflicts.
Fixed
CI/CD pipeline of the executable generation.
Merge issues after `dev` was merged back into `main`.

[1.0.0] - First Release (v1)
Added
Automatic pull request + clang-format in the CI.
Changed
Second AI model slot renamed to "Charter v1".
Fixed
Crash when reopening the plugin window repeatedly.
TOCTOU race condition in the startup profile resync thread.
clang-format auto fixes (several iterations).

[0.0.8] - Model Choice, Network Errors and License
Added
Model selection in the UI, sent along with the generation request.
Network error exception with a dedicated message.
MIT license.
Fixed
Model sent in the backend call.
Server URL.
Network error message.
CMake build on macOS.
CI/CD deploy test workflow.

[0.0.7] - Exceptions, Guest Mode and Multi-Instances
Added
Exceptions and cleaner error messages.
Colors and guest mode.
Support for multiple plugin instances at the same time.
Fixed
Workflow deploy VPS (many iterations).

[0.0.6] - 20 Parameters, macOS and VPS Deployment
Added
macOS version with 20 parameters, merged into the main branch.
Deployment of the VST3 on the VPS.
Documentation and README fixes.
Changed
Parameters and visuals reworked (branch `malo-macos`).
Fixed
Usage of strings, error message displayed on empty prompt, knots size.
Local session update (branch `refacto_param`).
Waveform selector changing randomly.
.gitignore.
CI/CD refactoring (several iterations).

[0.0.5] - Standalone / VST3 Build and Architecture Refactoring
Added
Compilation to VST3 and standalone executable.
CI/CD checking that the VST3 and the exe compile, standalone tests, Doxygen.
Wrapper to support both the exe and the plugin.
Config file and strings file.
Asynchronous backend calls.
Changed
Only the VST3 is built, since it already builds an exe by default; synth works in standalone and in FL Studio.
Main component split into several classes: Theme and Color manager, AI manager.
Backend now hosted on Railway.
README translated to English.

[0.0.4] - Themes, Layouts and Waveform Visual
Added
Themes in the app (first not saved in the database).
4 different layouts (including oscillators on the right).
Visual of the waveform.
Layout and theme stored in the user session and updated from the "get profile me" route.
Theme and layout update synced with the backend.
Doc comments and Doxygen CI.
Fixed
Layout with oscillators on the right.
CI/CD (many test iterations).

[0.0.3] - Authentication and Custom Errors
Added
Sign up and sign in.
Custom error messages, linked with the backend.
.gitignore.
Fixed
Class comments and README.

[0.0.2] - Generation Flow
Added
Components created and split into multiple files.
Parameters and Generate button.
Multiple options and prompt to generate the JSON.
Loading of a preset file.
Route to request the AI and load the generated preset, export file button, and synth keyboard (not fully functional yet).
CI/CD standalone app test.
Fixed
Display of the Generate and Load buttons.
Text button.
JUCE re-added as a proper git submodule.

[0.0.1] - Project Base and CI/CD
Added
Project base.
Window size set through a JSON config.
CI/CD, with the JUCE submodule and a branch mirror push.
Fixed
Executable path and CI/CD workflow (many iterations).