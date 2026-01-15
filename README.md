# Compilation (Windows)

Application standalone C++ basée sur JUCE 8.0.6, build avec CMake + Visual Studio.

## 1) Cloner le projet (avec submodules)
git clone --recurse-submodules https://github.com/Harmonia-EIP/Harmonia-App.git
cd Harmonia-App

## 2) Générer les fichiers de build
cmake -S . -B build -G "Visual Studio 17 2022" -A x64

## 3) Compiler (Release)
cmake --build build --config Release

## 4) Lancer l’exécutable
.\build\Release\HarmoniaApp.exe
