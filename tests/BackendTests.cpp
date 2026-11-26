#include <catch2/catch_test_macros.hpp>

#include "backend/core/JsonUtil.h"
#include "backend/net/ApiError.h"
#include "backend/storage/PaletteMapper.h"
#include "backend/storage/SessionMapper.h"

using json = nlohmann::json;

// ============================================================
// JsonUtil : jamais d'exception (une exception = crash du DAW)
// ============================================================

TEST_CASE ("JsonUtil: types inattendus et null renvoient la valeur de repli", "[backend][json]")
{
    const auto j = json::parse (R"({ "s": "ok", "n": 3, "b": true, "nul": null, "wrong": [1, 2] })");

    CHECK (JsonUtil::getString (j, "s") == "ok");
    CHECK (JsonUtil::getString (j, "nul", "fb") == "fb");
    CHECK (JsonUtil::getString (j, "n", "fb") == "fb");
    CHECK (JsonUtil::getString (j, "absent", "fb") == "fb");

    CHECK (JsonUtil::getInt (j, "n") == 3);
    CHECK (JsonUtil::getInt (j, "s", -1) == -1);
    CHECK (JsonUtil::getInt (j, "nul", -1) == -1);

    CHECK (JsonUtil::getBool (j, "b"));
    CHECK_FALSE (JsonUtil::getBool (j, "wrong"));

    // pas un objet : repli partout
    const auto arr = json::array();
    CHECK (JsonUtil::getString (arr, "s", "fb") == "fb");
    CHECK (JsonUtil::getInt (arr, "n", 7) == 7);
}

TEST_CASE ("JsonUtil: parseObject refuse ce qui n'est pas un objet JSON", "[backend][json]")
{
    CHECK (JsonUtil::parseObject (R"({ "a": 1 })").has_value());
    CHECK_FALSE (JsonUtil::parseObject ("<html>502 Bad Gateway</html>").has_value());
    CHECK_FALSE (JsonUtil::parseObject ("").has_value());
    CHECK_FALSE (JsonUtil::parseObject ("[1, 2]").has_value());
}

TEST_CASE ("JsonUtil: toJuce garde l'UTF-8", "[backend][json]")
{
    // "Noé ✦" en octets UTF-8 (pas de littéral accentué : MSVC lit le source en Windows-1252)
    CHECK (JsonUtil::toJuce ("No\xc3\xa9 \xe2\x9c\xa6").length() == 5);
}

// ============================================================
// SessionMapper
// ============================================================

TEST_CASE ("Session: réponse de login complète", "[backend][session]")
{
    const auto body = json::parse (R"({
        "token": "abc", "user_id": 42, "username": "noe", "email": "n@x.io", "avatar_url": null,
        "palette": [ { "id": 1, "color": "#111111" }, { "id": 2, "color": "#222222" }, { "id": 3, "color": "#333333" } ],
        "last_color_id": 2
    })");

    const auto s = SessionMapper::fromAuthResponse (body);

    REQUIRE (s.has_value());
    CHECK_FALSE (s->isGuest);
    CHECK (s->userId == 42);
    CHECK (s->accessToken == "abc");
    CHECK (s->pseudo == "noe");
    CHECK (s->avatarUrl.isEmpty());
    CHECK (s->paletteColours == juce::StringArray { "#111111", "#222222", "#333333" });
    CHECK (s->paletteSlot == 1);
    CHECK (s->expiresAt > juce::Time::getCurrentTime());
}

TEST_CASE ("Session: login sans token refusé", "[backend][session]")
{
    CHECK_FALSE (SessionMapper::fromAuthResponse (json::parse (R"({ "user_id": 1 })")).has_value());
    CHECK_FALSE (SessionMapper::fromAuthResponse (json::parse (R"({ "token": "" })")).has_value());
    CHECK_FALSE (SessionMapper::fromAuthResponse (json::parse (R"({ "token": null })")).has_value());
    CHECK_FALSE (SessionMapper::fromAuthResponse (json::array()).has_value());
}

TEST_CASE ("Session: aller-retour fichier", "[backend][session]")
{
    UserSession s;
    s.isGuest        = false;
    s.userId         = 7;
    s.pseudo         = juce::String::fromUTF8 ("No\xc3\xa9");
    s.email          = "n@x.io";
    s.accessToken    = "tok";
    s.expiresAt      = juce::Time (1'800'000'000'000LL);
    s.paletteColours = { "#00D4FF", "#7B8CFF", "#9AA5B8" };
    s.paletteSlot    = 2;

    const auto back = SessionMapper::fromFileJson (SessionMapper::toFileJson (s));

    REQUIRE (back.has_value());
    CHECK (back->userId == 7);
    CHECK (back->pseudo == s.pseudo);
    CHECK (back->accessToken == "tok");
    CHECK (back->expiresAt == s.expiresAt);
    CHECK (back->paletteColours == s.paletteColours);
    CHECK (back->paletteSlot == 2);
}

TEST_CASE ("Session: fichier invalide ou incomplet", "[backend][session]")
{
    // utilisateur sans token : refusé ; invité sans token : accepté
    CHECK_FALSE (SessionMapper::fromFileJson (json::parse (R"({ "isGuest": false })")).has_value());
    CHECK (SessionMapper::fromFileJson (json::parse (R"({ "isGuest": true })")).has_value());

    // palette incomplète oubliée, slot ramené dans 0..2
    const auto s = SessionMapper::fromFileJson (json::parse (
        R"({ "accessToken": "t", "paletteColours": ["#000000"], "paletteSlot": 9 })"));
    REQUIRE (s.has_value());
    CHECK (s->paletteColours.isEmpty());
    CHECK (s->paletteSlot == 2);
}

TEST_CASE ("Session: GET /profile/me met à jour sans toucher au token", "[backend][session]")
{
    UserSession s;
    s.accessToken = "keep";
    s.pseudo      = "old";
    s.avatarUrl   = "https://old";

    SessionMapper::applyProfileResponse (json::parse (R"({ "username": "new", "avatar_url": null })"), s);

    CHECK (s.accessToken == "keep");
    CHECK (s.pseudo == "new");
    CHECK (s.avatarUrl.isEmpty());   // avatar supprimé ailleurs : il disparaît ici aussi
}

// ============================================================
// PaletteMapper
// ============================================================

TEST_CASE ("Palette: validation des couleurs", "[backend][palette]")
{
    CHECK (PaletteMapper::isValidHex ("#00d4FF"));
    CHECK_FALSE (PaletteMapper::isValidHex ("00D4FF"));
    CHECK_FALSE (PaletteMapper::isValidHex ("#00D4F"));
    CHECK_FALSE (PaletteMapper::isValidHex ("#00D4FG"));

    CHECK (PaletteMapper::isValidPalette ({ "#000000", "#FFFFFF", "#123456" }));
    CHECK_FALSE (PaletteMapper::isValidPalette ({ "#000000", "#FFFFFF" }));
    CHECK_FALSE (PaletteMapper::isValidPalette ({ "#000000", "#FFFFFF", "red" }));

    CHECK (PaletteMapper::toHex (PaletteMapper::fromHex ("#7B8CFF")) == "#7B8CFF");
}

TEST_CASE ("Palette: lecture tolérante, rien n'est écrasé si invalide", "[backend][palette]")
{
    juce::StringArray colours { "#AAAAAA", "#BBBBBB", "#CCCCCC" };
    int slot = 1;

    // un id manquant (le 3) : la palette actuelle est gardée
    PaletteMapper::readFromJson (json::parse (
        R"({ "palette": [ { "id": 1, "color": "#111111" }, { "id": 2, "color": "#222222" }, { "id": 2, "color": "#333333" } ] })"),
        colours, slot);
    CHECK (colours[0] == "#AAAAAA");
    CHECK (slot == 1);

    // ids dans le désordre : remis dans l'ordre 1..3 ; last_color_id hors plage ramené à 0..2
    PaletteMapper::readFromJson (json::parse (
        R"({ "palette": [ { "id": 3, "color": "#333333" }, { "id": 1, "color": "#111111" }, { "id": 2, "color": "#222222" } ],
             "last_color_id": 99 })"),
        colours, slot);
    CHECK (colours == juce::StringArray { "#111111", "#222222", "#333333" });
    CHECK (slot == 2);
}

TEST_CASE ("Palette: corps du PUT (slot front 0..2 -> id backend 1..3)", "[backend][palette]")
{
    const auto body = PaletteMapper::toPutBody ({ "#111111", "#222222", "#333333" }, 0);

    CHECK (body["last_color_id"] == 1);
    REQUIRE (body["colors"].size() == 3);
    CHECK (body["colors"][2]["id"] == 3);
    CHECK (body["colors"][2]["color"] == "#333333");
}

// ============================================================
// ApiError : chaque réponse d'erreur donne un message affichable
// ============================================================

namespace
{
    HttpResponse response (int status, const std::string& body)
    {
        HttpResponse r;
        r.statusCode = status;
        r.body = body;
        return r;
    }
}

TEST_CASE ("ApiError: erreur réseau", "[backend][errors]")
{
    HttpResponse r;
    r.networkError = true;

    const auto e = parseApiError (r);
    CHECK (e.kind == ApiError::Kind::Network);
    CHECK (e.message.isNotEmpty());
}

TEST_CASE ("ApiError: codes backend connus", "[backend][errors]")
{
    const auto expired = parseApiError (response (401, R"({ "error_code": "TOKEN_EXPIRED", "detail": "jwt exp" })"));
    CHECK (expired.kind == ApiError::Kind::Unauthorized);
    CHECK (expired.code == "TOKEN_EXPIRED");
    CHECK (expired.detail == "jwt exp");
    CHECK (expired.message.isNotEmpty());
    CHECK_FALSE (expired.message.contains ("jwt"));   // le détail technique reste dans les logs

    const auto creds = parseApiError (response (400, R"({ "error_code": "INVALID_CREDENTIALS" })"));
    CHECK (creds.kind == ApiError::Kind::Http);
    CHECK (creds.message.isNotEmpty());
}

TEST_CASE ("ApiError: validation précise grâce à 'fields'", "[backend][errors]")
{
    const auto shortPwd = parseApiError (response (422, R"({
        "error_code": "VALIDATION_ERROR",
        "fields": [ { "field": "password", "type": "string_too_short", "ctx": { "min_length": 8 } } ] })"));
    CHECK (shortPwd.kind == ApiError::Kind::Validation);
    CHECK (shortPwd.message.contains ("8"));

    const auto generic = parseApiError (response (422, R"({ "error_code": "VALIDATION_ERROR" })"));
    CHECK (generic.kind == ApiError::Kind::Validation);
    CHECK (generic.message.isNotEmpty());
}

TEST_CASE ("ApiError: corps illisible (proxy, HTML)", "[backend][errors]")
{
    CHECK (parseApiError (response (502, "<html>Bad Gateway</html>")).kind == ApiError::Kind::Server);
    CHECK (parseApiError (response (401, "")).kind == ApiError::Kind::Unauthorized);

    const auto other = parseApiError (response (418, "teapot"));
    CHECK (other.kind == ApiError::Kind::Http);
    CHECK (other.message.isNotEmpty());

    CHECK (makeInvalidResponseError (200).kind == ApiError::Kind::Parse);
}
