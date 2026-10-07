/**
 * @file BackendManager.h
 * @brief Façade du backend : point d'entrée unique pour l'UI.
 *
 * L'API publique est identique à l'ancienne version : l'UI n'a pas à changer.
 * Tout le travail est délégué à :
 *   - AuthService     login / signup / logout
 *   - ProfileService  profil, sync de session, palette
 *   - AiService       génération / affinage de presets
 * qui utilisent HttpClient (réseau), SessionStore (fichier de session) et
 * BackendLogger (logs), et ne connaissent jamais BackendManager.
 */

#pragma once

#include <juce_core/juce_core.h>

#include <memory>
#include <optional>

#include "BackendTypes.h"
#include "core/BackendLogger.h"
#include "net/HttpClient.h"
#include "storage/SessionStore.h"

// Shim de compatibilité (voir le fichier) : l'ancien BackendManager.h incluait les managers.
#include "BackendAuthManager.h"

class AuthService;
class ProfileService;
class AiService;

class BackendManager
{
public:
    BackendManager();
    ~BackendManager();

    BackendManager (const BackendManager&) = delete;
    BackendManager& operator= (const BackendManager&) = delete;

    // =========================================================
    // AUTHENTIFICATION
    // =========================================================

    /** Login. En cas de succès, la session est sauvegardée et la resync du profil part en arrière-plan. */
    AuthResult loginUser (const juce::String& usernameOrEmail,
                          const juce::String& password);

    /** Création de compte. Même comportement que loginUser en cas de succès. */
    AuthResult signupUser (const juce::String& username,
                           const juce::String& firstname,
                           const juce::String& lastname,
                           const juce::String& email,
                           const juce::String& password);

    std::optional<UserSession> loadSession();
    void saveSession (const UserSession& session);

    /** Supprime la session locale. Appeler flushPaletteIfPending() AVANT. */
    void clearSession();

    // =========================================================
    // IA
    // =========================================================

    /** Retourne le JSON brut (format charter), à passer au PresetLoader. Invités refusés. */
    AiResult generatePreset (const juce::String& prompt, int modelId, const juce::String& backendName);

    AiResult refinePreset (const juce::String& prompt,
                           const juce::String& currentJson,
                           const juce::StringArray& lockedParamIds,
                           int modelId,
                           const juce::String& backendName);

    // =========================================================
    // PROFIL
    // =========================================================

    ProfileResult getProfile();

    /** Resync de la session depuis GET /profile/me, sur un thread de fond. */
    void syncProfileParamsInBackground (const UserSession& session);

    /** Resync bloquante. nullopt si échec. */
    std::optional<UserSession> syncProfileParams (const UserSession& session);

    /** Palette, bloquant. Refuse invités et palettes invalides. */
    ProfileResult updatePalette (const juce::StringArray& colours, int slot);

    /** Palette, en arrière-plan (debounce 2,5 s, ignore les PUT inutiles, invités ignorés). */
    void updatePaletteAsync (const juce::StringArray& colours, int slot);

    /** Envoie le changement de palette en attente, sans bloquer. */
    void flushPaletteIfPending();

    /** Définit l'état "connu du serveur" (palette de la session, vide pour un invité). */
    void resetPaletteSyncState (const juce::StringArray& colours, int slot);

    // =========================================================
    // CONFIGURATION / LOGS
    // (détails internes : à retirer de l'API publique une fois que l'UI ne les utilise plus,
    //  cf. étape 4.2 du plan)
    // =========================================================

    const juce::String& getApiUrl() const;
    const juce::File&   getSessionFile() const;
    const juce::File&   getLogFile() const;

    void writeLog (const juce::String& message) const;

private:
    // L'ordre de déclaration compte : les membres sont détruits en sens inverse,
    // donc les services (en dernier) disparaissent avant ce qu'ils utilisent.
    juce::SharedResourcePointer<BackendLogger> logger;
    juce::SharedResourcePointer<SessionStore>  sessionStore;
    HttpClient                                 http;

    std::unique_ptr<AuthService>    authService;
    std::unique_ptr<ProfileService> profileService;
    std::unique_ptr<AiService>      aiService;
};
