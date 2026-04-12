# Harmonia App – Frontend JUCE

Epitech Innovation Project (EIP)

Harmonia App est une application desktop développée en **C++ (C++17) avec JUCE**, servant d’interface utilisateur pour le projet Harmonia.
Ce dépôt représente le **frontend applicatif**, conçu pour intégrer des fonctionnalités audio avancées (DSP, synthèse, plugins).

---

## 🔗 Links

* 📦 GitHub Repository: https://github.com/Harmonia-EIP/Harmonia-App
* 📚 Documentation: https://harmonia-eip.github.io/Harmonia-App/

---

## 🚀 Features

* 🖥️ Interface desktop moderne (JUCE)
* 🎵 Base pour intégration audio (DSP, synthèse, VST)
* 🧩 Architecture modulaire et extensible
* ⚙️ Build cross-platform via CMake (Windows ready)

---

## 🎓 Contexte académique

* Projet réalisé dans le cadre de l’**Epitech Innovation Project (EIP)**
* Travail en équipe pluridisciplinaire
* Objectif : créer une application innovante combinant **UI + audio processing**

---

## 🛠️ Technologies utilisées

* **C++17**
* **JUCE 8.x**
* **CMake**
* **Visual Studio 2022**

---

## 📋 Prérequis

* Windows 10+
* Git (avec submodules)
* CMake ≥ 3.21
* Visual Studio 2022 (C++ workload)

---

## ⚡ Quick Start

```bash id="kqf0ot"
git clone --recurse-submodules https://github.com/Harmonia-EIP/Harmonia-App.git
cd Harmonia-App

cmake -S . -B build
cmake --build build --config Release
```

---

## 📦 Installation

### Clone avec submodules

```bash id="c0v5rq"
git clone --recurse-submodules https://github.com/Harmonia-EIP/Harmonia-App.git
cd Harmonia-App
```

### Si déjà cloné

```bash id="t6v6y8"
git submodule update --init --recursive
```

---

## 📁 Project Structure

```text id="c9c2qz"
Harmonia-App/
├─ Source/            # Code C++ (UI, logique, audio)
├─ JUCE/              # Framework JUCE (submodule)
├─ CMakeLists.txt     # Configuration CMake
├─ docs/
│   └── component_diagram.png
└─ build/             # Dossier de build (généré)
```

---

## 🏗️ Build & Run (Windows)

### Génération du projet

```bash id="sl7b06"
cmake -S . -B build
```

### Compilation

```bash id="5gzt6n"
cmake --build build --config Release
```

### Exécution

```bash id="s1ikc5"
build/Release/HarmoniaApp.exe
```

---

## 🧩 Architecture

### Component Diagram

![Component Diagram](assets/doc/diagram-components.png)

---

## 🎯 Objectifs du frontend

* Fournir une **interface utilisateur claire et moderne**
* Servir de base pour les **traitements audio avancés**
* Permettre une **évolution vers plugins (VST) et modules audio**

---

## ⚠️ Notes importantes

* Le dossier `build/` est **généré automatiquement**
* JUCE est géré via **Git submodule**
* Éviter toute modification directe de JUCE
* Projet conçu pour évoluer vers :

  * DSP avancé
  * Synthèse audio
  * Plugins VST

---

## 👥 Équipe

Projet développé dans le cadre de l’Epitech Innovation Project (EIP)

---

## 📝 Licence

Projet académique
Licence à définir selon l’évolution du projet
