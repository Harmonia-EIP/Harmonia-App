# Instructions de compilation du projet JUCE (Windows)

Ce projet est une application standalone écrite en C++, utilisant JUCE 8.0.6 avec :
- CMake (système de build)
- vcpkg (gestionnaire de paquets)
- Compilation Visual Studio (via CMake)
- vcpkg en **mode manifeste** (`vcpkg.json`)

---

## 1. Cloner le projet (avec les submodules)

```bash
git clone --recurse-submodules https://github.com/TON_USER/TON_REPO.git
cd TON_REPO

## 2. Installer vcpkg et installer les dépendances (JUCE)

cd vcpkg
.\bootstrap-vcpkg.bat
cd ..
.\vcpkg\vcpkg install


## 3. Set le chemin vers vcpkg dans l'env

$env:CMAKE_TOOLCHAIN_FILE = ".\vcpkg\scripts\buildsystems\vcpkg.cmake"


## 4. Génerer les fichiers de build

cmake -B build -S .


## 5. Compiler le projet

cmake --build build --config Release


## 6. Lancer l'exectuable

.\build\MyJUCEApp_artefacts\Release\HarmoniaApp.exe
