#ifndef LABEL_PROCESSING_H
#define LABEL_PROCESSING_H

#include <vector>

#include "nlohmann_json/json.hpp"

using Json = nlohmann::json;

namespace key_names
{
const std::string docsKey = "docs";
const std::string labelsKey = "labels";
const std::string labelType = "serviceName";
}

std::vector<Json> findNerLabels(const Json& labelsArray)
{
  assert(labelsArray.is_array());

  std::vector<Json> nerLabels;
  if (labelsArray.empty())
    return nerLabels;

  for (const auto& [key, label] : labelsArray.items())
  {
    if (!label.is_object() || !label.contains(key_names::labelType))
      continue;

    auto labelType = label.at(key_names::labelType);
    if (labelType != "NER")
      continue;

    nerLabels.push_back(label);
  }

  return nerLabels;
}

Json lemmatizeNerLabel(const Json& nerLabel)
{
  return nerLabel;
}

std::vector<Json> lemmatizeNerLabels(const std::vector<Json>& nerLabels)
{
  std::vector<Json> lemmatizedLabels;
  for (const auto& nerLabel : nerLabels)
    lemmatizedLabels.push_back(lemmatizeNerLabel(nerLabel));
  return lemmatizedLabels;
}

void addLemmatizedLabels(Json& targetLabelsArray, const std::vector<Json>& lemmatizedLabels)
{
  assert(targetLabelsArray.is_array());
  for (const auto& lemmatizedLabel : lemmatizedLabels)
    targetLabelsArray.push_back(lemmatizedLabel);
}

#endif // LABEL_PROCESSING_H
