#pragma once

enum class AIModel { Model1 = 1, Model2, Model3 };

struct AIModelData {
  AIModel id;
  const char *name;
  const char *backendname;
};

static constexpr AIModelData aiModels[] = {
    {AIModel::Model1, "Model 1", "model-1"},
    {AIModel::Model2, "Charter v1", "charter_v1"},
    {AIModel::Model3, "Harmonia v2", "harmonia_v2"}};