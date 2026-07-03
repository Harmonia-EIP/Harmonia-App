#pragma once

enum class AIModel
{
    Model1 = 1,
    Model2
};

struct AIModelData
{
    AIModel id;
    const char* name;
    const char* backendname;
};

static constexpr AIModelData aiModels[] =
{
    { AIModel::Model1, "Model 1", "model-1" },
    { AIModel::Model2, "Model 2", "model-2" }
};