# Test Report - Harmonia Plugin

**Execution Date:** 09/11/2026
**Target Environment:** Windows PC, FL Studio (DAW)[cite: 1]

## Test Execution Results

| ID | Module | Test Scenario | Expected Result | Actual Result | Status | Ticket |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **INST-01** | Installation | Download the plugin from the website[cite: 1] | The package downloads completely without corruption. | Download successful[cite: 1] | ✅ SUCCESS | - |
| **INST-02** | Installation | Unzip the downloaded package[cite: 1] | The package extracts to reveal the plugin files. | Package unzipped[cite: 1] | ✅ SUCCESS | - |
| **INST-03** | Installation | Place the `.vst3` file into the local plugin folder (`C:\Program Files\Common Files\VST3` by default for FL Studio)[cite: 1] | The file is successfully moved to the correct directory. | File placed correctly[cite: 1] | ✅ SUCCESS | - |
| **DAW-01** | Integration | Scan for the plugin inside the DAW[cite: 1] | The DAW detects the new plugin and allows it to be opened. | The plugin is scanned and opens immediately[cite: 1] | ✅ SUCCESS | - |
| **AUTH-01** | Account | Create a user account[cite: 1] | Account creation process completes without errors. | Account created with no problems[cite: 1] | ✅ SUCCESS | - |
| **CORE-01** | Features | Generate a preset[cite: 1] | The plugin successfully generates a new audio preset. | Preset generation is OK[cite: 1] | ✅ SUCCESS | - |
