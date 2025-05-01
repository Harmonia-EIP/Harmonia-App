#include "MainComponent.h"
#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>

MainComponent::MainComponent()
{
    int width = 400;
    int height = 300;

    try {
        // Chemin absolu à partir du fichier source
        std::filesystem::path configPath = std::filesystem::path(__FILE__).parent_path() / "config" / "config.json";

        std::ifstream i(configPath);
        if (!i) throw std::runtime_error("Fichier introuvable");

        nlohmann::json j;
        i >> j;

        width = j["window"].value("width", width);
        height = j["window"].value("height", height);
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors du chargement de config.json : " << e.what() << "\n";
    }

    setSize(width, height);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkslategrey);
    g.setColour (juce::Colours::white);
    g.setFont (30.0f);
    g.drawFittedText ("Hello JUCE!", getLocalBounds(), juce::Justification::centred, 1);
}
