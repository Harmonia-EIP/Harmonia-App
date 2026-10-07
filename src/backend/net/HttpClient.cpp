#include "HttpClient.h"

#include <cpr/cpr.h>

namespace
{
    enum class Method { Get, Post, Put };

    HttpResponse convert (const cpr::Response& r)
    {
        HttpResponse out;
        out.statusCode = (int) r.status_code;
        out.body       = r.text;

        if (r.error.code != cpr::ErrorCode::OK)
        {
            out.networkError        = true;
            out.networkErrorCode    = (int) r.error.code;
            out.networkErrorMessage = juce::String (r.error.message);
        }

        return out;
    }

    HttpResponse send (Method method,
                       const juce::String& baseUrl,
                       const juce::String& path,
                       const std::string& body,
                       const HttpOptions& options)
    {
        const cpr::Url url { (baseUrl + path).toStdString() };

        cpr::Header headers { { "Content-Type", "application/json" } };

        if (options.bearerToken.isNotEmpty())
            headers["Authorization"] = "Bearer " + options.bearerToken.toStdString();

        const cpr::Timeout        timeout        { options.timeoutMs };
        const cpr::ConnectTimeout connectTimeout { options.connectTimeoutMs };

        switch (method)
        {
            case Method::Get:
                return convert (cpr::Get (url, headers, timeout, connectTimeout));

            case Method::Post:
                return convert (cpr::Post (url, headers, cpr::Body { body }, timeout, connectTimeout));

            case Method::Put:
                return convert (cpr::Put (url, headers, cpr::Body { body }, timeout, connectTimeout));
        }

        return {};
    }
}

HttpClient::HttpClient (const juce::String& base)
    : baseUrl (base)
{
}

HttpResponse HttpClient::get (const juce::String& path, const HttpOptions& options) const
{
    return send (Method::Get, baseUrl, path, {}, options);
}

HttpResponse HttpClient::post (const juce::String& path, const std::string& jsonBody, const HttpOptions& options) const
{
    return send (Method::Post, baseUrl, path, jsonBody, options);
}

HttpResponse HttpClient::put (const juce::String& path, const std::string& jsonBody, const HttpOptions& options) const
{
    return send (Method::Put, baseUrl, path, jsonBody, options);
}
