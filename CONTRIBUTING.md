# Contributing to [Project Name]

First off, thank you for considering contributing to this project! Whether you are fixing a bug, adding a new feature, or improving the documentation, your help is greatly appreciated.

## 🎯 Target Platforms & Technical Stack

To ensure the stability of the project, all contributions must be compatible with our core stack and tested on the targeted platforms.

### Supported Environment
* **Primary OS / IDE:** Windows 10/11 using **Visual Studio 2022**
* **Build System:** CMake

### Core Technologies & Dependencies
* **Language:** C++17
* **Framework:** JUCE 8
* **External Libraries:** 
  * CPR (C++ Requests - HTTP Client)
  * nlohmann/json

---

## 🚀 Getting Started (Welcome Tickets)

If you are new to the project or looking for a way to jump in, we highly recommend starting with our welcome tickets.
1. Go to the **Issues** tab of this repository.
2. Filter by the `good first issue` or `help wanted` labels.
3. Assign the issue to yourself and leave a comment to let others know you are working on it.

---

## 🛠️ Contribution Workflow

Here is the step-by-step process to submit your code:

### 1. Fork and Clone
Fork the repository to your own GitHub account, then clone it locally:
```bash
git clone git@github.com:Harmonia-EIP/Harmonia-App.git
```

### 2. Create a Branch
We do not enforce strict branch naming conventions, but please use descriptive names that reflect your work:
```bash
git checkout -b add-login-button
# or
git checkout -b fix-audio-glitch
```

### 3. Commit your Changes
We strictly follow our **Conventional Commits** specification. Please format your commit messages accordingly to keep the history clean and readable:
* `[ADD]` for new features (e.g., `feat: add HTTP client using CPR`)
* `[FIX]` for bug fixes (e.g., `fix: resolve crash on JSON parse`)
* `[DOCS]:` for documentation updates
* `[REFACTO]` for code refactoring without adding features or fixing bugs

### 4. Push and Open a Pull Request (PR)
Push your branch to your fork:
```bash
git push origin your-branch-name
```
Open a Pull Request against our `main` branch. Please provide a clear description of the changes and link any related issues (e.g., "Closes #5").

---

## 🧪 Testing & Quality Assurance

Before submitting your Pull Request, please ensure that:
1. The project compiles successfully via CMake and Visual Studio 2022.
2. You have tested your changes according to our test cases.
3. If you introduce a new module, you include the corresponding test cases in our Testing Report.
