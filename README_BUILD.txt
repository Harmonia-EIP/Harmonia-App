# Instructions de compilation du projet JUCE (Windows)

Ce projet est une application standalone écrite en C++, utilisant JUCE 8.0.6 avec :
- CMake (système de build)
- Compilation Visual Studio (via CMake)

---

## 1. Cloner le projet (avec les submodules)

git clone --recurse-submodules https://github.com/TON_USER/TON_REPO.git
cd TON_REPO

## 2. Génerer les fichiers de build

cmake -B build -S .


## 3. Compiler le projet

cmake --build build --config Release


## 4. Lancer l'exectuable

.\build\Release\HarmoniaApp.exe