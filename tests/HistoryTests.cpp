#include <catch2/catch_test_macros.hpp>

#include "backend/storage/HistoryMapper.h"
#include "sections/header_components/HistoryUtils.h"

TEST_CASE ("Historique: model_name / model_id sont lus, null compris", "[history]")
{
    const auto body = nlohmann::json::parse (R"({
        "history": [
            { "id": 1, "name": "Pad", "type": "generate", "model_id": null, "model_name": "harmonia_v3",
              "created_at": "2026-11-25T12:00:00Z", "last_used_at": "2026-11-25T12:10:00Z",
              "parameters": { "osc_mix": 0.5 } },
            { "id": 2, "name": null, "type": "refine", "model_id": null, "model_name": null,
              "created_at": "2026-11-25T12:00:00Z", "last_used_at": "2026-11-25T12:10:00Z" }
        ]
    })");

    const auto entries = HistoryMapper::readFromJson (body);

    REQUIRE (entries.size() == 2);
    CHECK (entries[0].modelName == "harmonia_v3");
    CHECK (entries[0].modelId.isEmpty());
    CHECK (entries[0].parameters.at ("osc_mix") == 0.5f);
    CHECK (entries[1].id == 2);
    CHECK (entries[1].name.isEmpty());
    CHECK (entries[1].modelName.isEmpty());
}

TEST_CASE ("Historique: nom de modèle affiché de façon homogène", "[history]")
{
    AIModelList models;
    models.push_back ({ "charter_v1", "Charter v1 (legacy)", false });

    using HistoryUtils::formatModel;

    CHECK (formatModel ("charter_v1",  {}, models) == "Charter v1 (legacy)");   // nom du ModelPicker
    CHECK (formatModel ("harmonia_v3", {}, models) == "Harmonia V3");           // inconnu : mis en forme
    CHECK (formatModel ("latest",      {}, models) == "Latest");
    CHECK (formatModel ("LATEST",      {}, {})     == "Latest");
    CHECK (formatModel ({},  "charter_v1", models) == "Charter v1 (legacy)");   // repli sur model_id
    CHECK (formatModel ({},  {},           models) == "Unknown model");
    CHECK (formatModel ("  ", {},          models) == "Unknown model");
}
