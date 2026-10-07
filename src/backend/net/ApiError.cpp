#include "ApiError.h"

#include "../core/BackendStrings.h"
#include "../core/JsonUtil.h"

namespace
{
    using json = nlohmann::json;

    // detail = tableau d'erreurs de validation (format FastAPI / Pydantic) :
    // [{ "type": "...", "loc": ["body", "email"], "ctx": { "min_length": 8 } }, ...]
    juce::String validationMessage (const json& detail)
    {
        if (! detail.is_array() || detail.empty())
            return BackendStrings::InvalidInput;

        const auto& error = detail[0];
        const auto type   = JsonUtil::getString (error, "type");

        std::string field;

        if (error.is_object() && error.contains ("loc") && error["loc"].is_array())
            for (const auto& item : error["loc"])
                if (item.is_string() && item.get<std::string>() != "body")
                    field = item.get<std::string>();

        if (field == "email")
            return BackendStrings::InvalidEmail;

        if (field == "password" && type == "string_too_short")
        {
            int minLength = 0;

            if (error.contains ("ctx") && error["ctx"].is_object())
                minLength = JsonUtil::getInt (error["ctx"], "min_length", 0);

            return BackendStrings::passwordTooShort (minLength);
        }

        if (type == "missing")
            return BackendStrings::MissingField;

        return BackendStrings::InvalidInput;
    }
}

ApiError parseApiError (const HttpResponse& response)
{
    ApiError error;
    error.statusCode = response.statusCode;

    if (response.networkError)
    {
        error.kind    = ApiError::Kind::Network;
        error.message = Strings::Errors::NetworkError;
        return error;
    }

    // Un 5xx ne montre jamais le détail du serveur à l'utilisateur.
    if (response.statusCode >= 500)
    {
        error.kind    = ApiError::Kind::Server;
        error.message = BackendStrings::ServerError;
        return error;
    }

    juce::String detailText;

    if (const auto body = JsonUtil::parseObject (response.body))
    {
        const auto it = body->find ("detail");

        if (it != body->end())
        {
            if (it->is_string())
            {
                detailText = JsonUtil::toJuce (it->get<std::string>());
            }
            else if (it->is_array())
            {
                error.kind    = ApiError::Kind::Validation;
                error.message = validationMessage (*it);
                return error;
            }
        }
    }

    if (response.statusCode == 401)
    {
        error.kind    = ApiError::Kind::Unauthorized;
        error.message = detailText.isNotEmpty() ? detailText : BackendStrings::SessionExpired;
        return error;
    }

    error.kind    = ApiError::Kind::Http;
    error.message = detailText.isNotEmpty() ? detailText : Strings::Errors::UnknownError;
    return error;
}

ApiError makeInvalidResponseError (int statusCode)
{
    ApiError error;
    error.kind       = ApiError::Kind::Parse;
    error.message    = BackendStrings::InvalidServerResponse;
    error.statusCode = statusCode;
    return error;
}
