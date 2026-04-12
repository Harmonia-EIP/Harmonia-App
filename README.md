# Harmonia App – JUCE Frontend

Epitech Innovation Project (EIP)

Harmonia App is a desktop application developed in **C++ (C++17) using JUCE**, designed as the user interface for the Harmonia project.
This repository represents the **application frontend**, built to support advanced audio features such as DSP, synthesis, and plugins.

---

## 🔗 Links

* 📦 GitHub Repository: https://github.com/Harmonia-EIP/Harmonia-App
* 📚 Documentation: https://harmonia-eip.github.io/Harmonia-App/

---

## 🚀 Features

* 🖥️ Modern desktop user interface (JUCE)
* 🎵 Foundation for audio integration (DSP, synthesis, VST)
* 🧩 Modular and extensible architecture
* ⚙️ Cross-platform build system with CMake (Windows ready)

---

## 🎓 Academic Context

* Developed as part of the **Epitech Innovation Project (EIP)**
* Collaborative work in a multidisciplinary team
* Goal: build an innovative application combining **UI and audio processing**

---

## 🛠️ Technologies Used

* **C++17**
* **JUCE 8.0.8
* **CMake**
* **Visual Studio 2022**

---

## 📋 Requirements

* Windows 10+
* Git (with submodules)
* CMake ≥ 3.21
* Visual Studio 2022 (C++ workload installed)

---

## ⚡ Quick Start

```bash
git clone --recurse-submodules https://github.com/Harmonia-EIP/Harmonia-App.git
cd Harmonia-App

cmake -S . -B build
cmake --build build --config Release
```

---

## 📦 Installation

### Clone with submodules

```bash
git clone --recurse-submodules https://github.com/Harmonia-EIP/Harmonia-App.git
cd Harmonia-App
```

### If already cloned

```bash
git submodule update --init --recursive
```

---

## 📁 Project Structure

```text
Harmonia-App/
├─ Source/            # C++ source code (UI, logic, audio)
├─ JUCE/              # JUCE framework (submodule)
├─ CMakeLists.txt     # CMake configuration
├─ docs/
│   └── component_diagram.png
└─ build/             # Generated build directory
```

---

## 🏗️ Build & Run (Windows)

### Generate project

```bash
cmake -S . -B build
```

### Build

```bash
cmake --build build --config Release
```

### Run

```bash
build/Release/HarmoniaApp.exe
```

---

## 🧩 Architecture

### Component Diagram

![Component Diagram](assets/doc/diagram-components.png)

---

## 🎯 Frontend Goals

* Provide a **clean and modern user interface**
* Serve as a foundation for **advanced audio processing**
* Enable future expansion into **plugins (VST) and audio modules**

---

## ⚠️ Important Notes

* The `build/` directory is **auto-generated**
* JUCE is managed via a **Git submodule**
* Avoid modifying JUCE directly
* The project is designed to evolve towards:

  * Advanced DSP
  * Audio synthesis
  * VST plugins

---

## 👥 Team

Developed as part of the Epitech Innovation Project (EIP)

---

## 📝 License

Academic project
License to be defined as the project evolves
