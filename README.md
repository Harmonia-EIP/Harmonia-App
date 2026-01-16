# Harmonia App – Frontend JUCE  
Epitech Innovation Project (EIP)

Harmonia App est une application desktop développée en **C++ avec JUCE**, réalisée dans le cadre d’un **Epitech Innovation Project (EIP)**.  
Ce dépôt correspond au **frontend applicatif**, conçu pour servir d’interface utilisateur et de base technique pour les fonctionnalités audio du projet Harmonia.

---

## Contexte académique

- Projet réalisé dans le cadre de l’EIP – Epitech  
- Travail en équipe pluridisciplinaire  
- Objectif : concevoir une application innovante combinant interface graphique moderne et technologies audio  

---

## Technologies utilisées

- C++ (C++17)
- JUCE 8.x
- CMake
- Visual Studio 2022 (Windows)

---

## Prérequis

- Windows 10 ou supérieur  
- Git (avec gestion des submodules)  
- CMake ≥ 3.21  
- Visual Studio 2022 (workload C++ installé)

---

## Récupération du projet

Le projet utilise **JUCE comme submodule**.

```bash
git clone --recurse-submodules https://github.com/Harmonia-EIP/Harmonia-App.git
cd Harmonia-App
```

Si le dépôt a déjà été cloné sans les submodules :

```bash
git submodule update --init --recursive
```

---

## Structure du projet

```text
Harmonia-App/
├─ Source/            # Code C++ (UI, logique applicative, audio)
├─ JUCE/              # Framework JUCE (submodule)
├─ CMakeLists.txt     # Configuration CMake
└─ build/             # Dossier de build (généré)
```

---

## Compilation (Windows)

### Génération du projet Visual Studio

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
```

### Compilation en mode Release

```bash
cmake --build build --config Release
```

### Lancement de l’application

```bash
build/Release/HarmoniaApp.exe
```

---

## Notes importantes

- Le dossier `build/` peut être supprimé et régénéré à tout moment  
- JUCE est géré exclusivement via le submodule Git  
- Toute modification du framework doit être évitée ou documentée  
- Le projet est prévu pour évoluer vers des fonctionnalités audio avancées (DSP, synthèse, VST)

---

## Objectifs du frontend

- Fournir une interface utilisateur claire et moderne  
- Servir de base technique solide pour les traitements audio  
- Préparer une architecture extensible (standalone, plugins, modules futurs)

---

## Équipe

Projet développé dans le cadre de l’Epitech Innovation Project (EIP).

---

## Licence

Projet académique
Licence à définir selon l’évolution du projet.
