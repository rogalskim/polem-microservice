#include <cassert>

#include "label_processing.h"

#include <polem-dev/CascadeLemmatizer.h>

using Json = nlohmann::json;

namespace label_processing
{

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
  assert(nerLabel.is_object());
  assert(nerLabel.contains("value"));

  std::string inputValue = nerLabel["value"];

  CascadeLemmatizer lemmatizer = CascadeLemmatizer::assembleLemmatizer();
  auto lemma = lemmatizer.lemmatize("Alejach Jerozolimskich",
                                    "aleja jerozolimski",
                                    "subst:pl:loc:f adj:pl:loc:f:pos",
                                    false);
  Json lemmatizedLabel = nerLabel;
  std::string str;
  lemma.toUTF8String(str);
  lemmatizedLabel["value"] = str;
  return lemmatizedLabel;
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

}
