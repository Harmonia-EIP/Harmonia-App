#include "ApiError.h"

namespace
{
    using json = nlohmann::json;

    // fields (optionnel, si le backend l'envoie) :
    // [{ "field": "email", "type": "string_too_short", "ctx": { "min_length": 8 } }, ...]
    juce::String validationMessage (const json& fields)
    {
        if (! fields.is_array() || fields.empty() || ! fields[0].is_object())
            return BackendStrings::InvalidInput;

        const auto& f    = fields[0];
        const auto field = JsonUtil::getString (f, "field");
        const auto type  = JsonUtil::getString (f, "type");

        if (field == "email")
            return BackendStrings::InvalidEmail;

        if (field == "password" && type == "string_too_short")
        {
            int minLength = 0;

            if (f.contains ("ctx") && f["ctx"].is_object())
                minLength = JsonUtil::getInt (f["ctx"], "min_length", 0);

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

    const json* fields = nullptr;
    const auto body = JsonUtil::parseObject (response.body);

    if (body)
    {
        if (auto it = body->find ("error_code"); it != body->end() && it->is_string())
            error.code = JsonUtil::toJuce (it->get<std::string>());

        if (auto it = body->find ("detail"); it != body->end() && it->is_string())
            error.detail = JsonUtil::toJuce (it->get<std::string>());

        if (auto it = body->find ("fields"); it != body->end() && it->is_array())
            fields = &*it;
    }

    // 1) Validation : précise si "fields" est présent, générique sinon
    if (error.code == ErrorCodes::ValidationError)
    {
        error.kind    = ApiError::Kind::Validation;
        error.message = fields != nullptr ? validationMessage (*fields)
                                          : BackendStrings::InvalidInput;
        return error;
    }

    // 2) Code connu -> message traduit
    const auto mapped = BackendStrings::forErrorCode (error.code);

    if (mapped.isNotEmpty())
    {
        error.message = mapped;

        if (ErrorCodes::isTokenError (error.code))  error.kind = ApiError::Kind::Unauthorized;
        else if (response.statusCode >= 500)        error.kind = ApiError::Kind::Server;
        else                                        error.kind = ApiError::Kind::Http;

        return error;
    }

    // 3) Fallbacks : code absent ou inconnu (proxy, HTML, ancienne version...)
    if (response.statusCode >= 500)
    {
        error.kind    = ApiError::Kind::Server;
        error.message = BackendStrings::ServerError;
        return error;
    }

    if (response.statusCode == 401)
    {
        error.kind    = ApiError::Kind::Unauthorized;
        error.message = BackendStrings::SessionExpired;
        return error;
    }

    error.kind    = ApiError::Kind::Http;
    error.message = Strings::Errors::UnknownError;
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