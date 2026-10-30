#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <array>
#include <functional>
#include <vector>

#include "../themes/HarmoniaPalette.h"
#include "../themes/HiveLookAndFeel.h"
#include "../themes/PaletteSelector.h"
#include "../backend/BackendManager.h"
#include "../backend/BackendPalette.h"
#include "../backend/BackendTypes.h"
#include "../tools/Alert.h"
#include "../tools/AvatarLoader.h"
#include "../config/AiConfig.h"

#include "header_components/HeaderColours.h"
#include "header_components/PromptFont.h"
#include "header_components/IconButton.h"
#include "header_components/LogoButton.h"
#include "header_components/ActionButton.h"
#include "header_components/ModelMenuLookAndFeel.h"
#include "header_components/ModelPicker.h"
#include "header_components/HistoryPicker.h"
#include "header_components/PromptEditor.h"

// Label dont le tooltip est calculé à la demande : toujours à jour,
// et présent seulement si le texte est coupé par les "..."
class PresetLabel : public juce::Label
{
public:
    juce::String getTooltip() override
    {
        const float available = (float) (getWidth() - getBorderSize().getLeftAndRight());

        return getFont().getStringWidthFloat (getText()) > available ? getText()
                                                                     : juce::String();
    }
};

//==============================================================================
class HeaderComponent : public juce::Component,
                        private juce::Timer
{
public:
    enum class Mode { Generate, Refine };

    // Type de l'historique reçu du serveur (adapte "HistoryEntry" au nom réel
    // défini dans ProfileSnapshot.h)
    using HistoryList = std::vector<HistoryEntry>;

    // Même limite que MAX_HISTORY_PER_USER côté serveur
    static constexpr size_t maxHistoryEntries = 50;

    // initialModels : liste récupérée via GET /ai/models (peut être vide, on la
    // met à jour plus tard avec setModels()).
    HeaderComponent (const UserSession& session, AIModelList initialModels = {});
    ~HeaderComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

    void setLogo (const juce::Image& img) { logoButton.setLogo (img); }

    juce::Button& getGenerateButton() { return actionButton; }
    juce::Button& getLoadButton()     { return loadButton; }
    juce::Button& getSaveButton()     { return saveButton; }
    juce::Button& getLogoutButton()   { return homeButton; }
    juce::Button& getLogoButton()     { return logoButton; }

    juce::TextEditor& getPromptEditor()    { return promptEditor; }
    juce::Label&      getPresetLabel()     { return presetLabel; }
    PaletteSelector&  getPaletteSelector() { return paletteSelector; }

    Mode getMode() const { return mode; }
    void setMode (Mode m, bool notify = true);
    void restoreSlot (int slot);
    void reapplyTheme();

    void setAvatarUrl (const juce::String& url);

    // Remonte une entrée existante en tête de liste (mise à jour locale, sans réseau).
    void moveHistoryToTop (int id);

    // Insère une nouvelle entrée (créée par le serveur) en tête de liste, plafonnée à 50.
    void addHistoryEntry (const HistoryEntry& e);

    std::function<void (HarmoniaPalette::Theme)> onThemeChanged;
    std::function<void (Mode)> onModeChanged;

    // --- Historique ---------------------------------------------------------
    // Le header garde la liste complète et renvoie l'ENTRÉE complète (pas un index) :
    // le MainComponent n'a rien à stocker ni à comparer.

    // L'utilisateur a choisi une entrée dans le menu Historique.
    // Le MainComponent décide quoi en faire (charger les paramètres).
    std::function<void (const HistoryList::value_type&)> onHistorySelected;

    // L'utilisateur a supprimé une entrée (croix). Déjà retirée de la liste locale.
    // Le MainComponent doit la supprimer côté serveur.
    std::function<void (const HistoryList::value_type&)> onHistoryDeleted;

    // L'utilisateur a cliqué sur "Clear history". Déjà vidé en local.
    std::function<void()> onHistoryCleared;

    // Palette modifiée PAR L'UTILISATEUR (couleur ou pastille active) : 3 hex "#RRGGBB" + slot 0..2.
    // Jamais appelé pour un invité, ni quand la palette vient du serveur.
    std::function<void (const juce::StringArray&, int)> onPaletteChanged;

    // --- Données du profil (reçues du MainComponent, le header ne parle jamais au backend) ---

    // Reçoit l'historique des générations : le stocke, logue le nombre d'entrées
    // et remplit le menu déroulant Historique.
    void setHistory (const HistoryList& entries);

    // Reçoit la palette du serveur (3 hex "#RRGGBB" + slot actif 0..2).
    // Ignorée si la liste n'a pas exactement 3 couleurs ou si c'est un invité.
    // NE déclenche PAS onPaletteChanged (pas de renvoi inutile au serveur).
    void setPaletteFromServer (const juce::StringArray& hexColours, int activeSlot);

    // --- Modèles IA ---------------------------------------------------------
    // Remplace la liste (appelé quand GET /ai/models répond). Garde le choix de
    // l'utilisateur s'il existe toujours, sinon sélectionne le modèle "latest".
    void setModels (const AIModelList& newModels);

    // nullptr si invité ou liste vide.
    const AIModelData* getSelectedModel() const;

    juce::String getSelectedModelName() const;

    // Valeur à envoyer dans "model_name" : "latest" ou l'id du modèle choisi.
    juce::String getSelectedBackendName() const;

    void setPresetName (const juce::String& name);

private:

    juce::Image  defaultLogo;
    juce::String currentAvatarUrl;
    int          avatarRequestId = 0;

    void timerCallback() override;

    // Passe a true quand le Refine est pret cote IA
    bool refineEnabled = false;

    juce::Colour modeColour() const;
    void applyColours();
    void updateModeTexts();
    void paintPromptBar (juce::Graphics&);
    void rebuildModelPicker (const juce::String& keepId);

    UserSession session;
    AIModelList models;          // doit rester APRES `session` (ordre d'initialisation)

    HistoryList historyEntries;  // historique reçu du serveur (source de vérité)

    // true pendant l'application d'une palette venant du serveur : coupe onPaletteChanged
    bool applyingServerPalette = false;

    juce::Label titleLabel;
    PresetLabel presetLabel; 

    IconButton homeButton { IconButton::Icon::Home };
    IconButton loadButton { IconButton::Icon::Import };
    IconButton saveButton { IconButton::Icon::Export };

    LogoButton   logoButton;
    ActionButton actionButton;

    ModelPicker      modelPicker;
    HistoryPicker    historyPicker;
    PromptEditor     promptEditor;
    PaletteSelector  paletteSelector;

    juce::Rectangle<int> promptBounds, fileGroupBounds;

    Mode mode = Mode::Generate;
    float modeAnim = 0.0f, targetAnim = 0.0f;
    float focusAnim = 0.0f;   // 0 = barre sans focus, 1 = en cours de saisie
    float waveAnim  = 1.0f;   // 1 = égaliseur visible, 0 = caché (saisie en cours)
};