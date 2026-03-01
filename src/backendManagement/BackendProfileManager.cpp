#include "BackendProfileManager.h"
#include "BackendManager.h"
#include <cpr/cpr.h>

using json = nlohmann::json;

BackendProfileManager::BackendProfileManager(BackendManager& bm)
    : backend(bm)
{
}

ProfileResult BackendProfileManager::getProfile()
{
    ProfileResult result;

    auto sessionOpt = backend.loadSession();
    if (!sessionOpt.has_value())
    {
        result.success = false;
        result.errorMessage = "Pas de session active";
        return result;
    }

    auto& session = sessionOpt.value();

    auto url = backend.getApiUrl() + "/profile/me";
    auto response = cpr::Get(
        cpr::Url{ url.toStdString() },
        cpr::Header{
            { "Authorization", "Bearer " + session.accessToken.toStdString() },
            { "Content-Type", "application/json" }
        }
    );

    backend.writeLog("GET /profile/me : " + juce::String(response.status_code));
    backend.writeLog("Réponse : " + juce::String(response.text.c_str()));

    if (response.status_code != 200)
    {
        result.success = false;
        try
        {
            auto body = json::parse(response.text);
            if (body.contains("detail"))
                result.errorMessage = body["detail"].get<std::string>();
            else
                result.errorMessage = "Erreur inconnue";
        }
        catch (...)
        {
            result.errorMessage = "Impossible d'analyser la réponse du serveur";
        }
        return result;
    }

    try
    {
        auto body = json::parse(response.text);

        UserProfile profile;
        profile.id       = body.value("id", 0);
        profile.username = body.value("username", "");
        profile.email    = body.value("email", "");
        profile.firstName= body.value("first_name", "");
        profile.lastName = body.value("last_name", "");
        profile.createdAt= body.value("created_at", "");
        profile.role     = body.value("role", "");
        profile.isActive = body.value("is_active", true);
        profile.layoutId = body.value("layout_id", 1);
        profile.themeId  = body.value("theme_id", 1);

        result.success = true;
        result.profile = profile;
    }
    catch (...)
    {
        result.success = false;
        result.errorMessage = "Impossible de parser le profil";
    }

    return result;
}


ProfileResult BackendProfileManager::updateTheme(int themeId)
{
    ProfileResult result;

    auto sessionOpt = backend.loadSession();
    if (!sessionOpt.has_value())
    {
        result.success = false;
        result.errorMessage = "Pas de session active";
        return result;
    }

    auto& session = sessionOpt.value();

    json body;
    body["theme_id"] = themeId;

    auto url = backend.getApiUrl() + "/profile/" 
               + juce::String(session.userId) 
               + "/theme";

    auto response = cpr::Put(
        cpr::Url{ url.toStdString() },
        cpr::Header{
            { "Authorization", "Bearer " + session.accessToken.toStdString() },
            { "Content-Type", "application/json" }
        },
        cpr::Body{ body.dump() }
    );

    backend.writeLog("PUT /theme : " + juce::String(response.status_code));

    if (response.status_code != 200)
    {
        result.success = false;
        result.errorMessage = "Erreur update theme";
        return result;
    }

    result.success = true;
    return result;
}

ProfileResult BackendProfileManager::updateLayout(int layoutId)
{
    ProfileResult result;

    auto sessionOpt = backend.loadSession();
    if (!sessionOpt.has_value())
    {
        result.success = false;
        result.errorMessage = "Pas de session active";
        return result;
    }

    auto& session = sessionOpt.value();

    json body;
    body["layout_id"] = layoutId;

    auto url = backend.getApiUrl() + "/profile/" 
               + juce::String(session.userId) 
               + "/layout";

    auto response = cpr::Put(
        cpr::Url{ url.toStdString() },
        cpr::Header{
            { "Authorization", "Bearer " + session.accessToken.toStdString() },
            { "Content-Type", "application/json" }
        },
        cpr::Body{ body.dump() }
    );

    backend.writeLog("PUT /layout : " + juce::String(response.status_code));

    if (response.status_code != 200)
    {
        result.success = false;
        result.errorMessage = "Erreur update layout";
        return result;
    }

    result.success = true;
    return result;
}   