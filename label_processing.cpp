#include <cassert>
#include <exception>
#include <map>

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
    if (!label.is_object() || !label.contains(key_names::labelService))
      continue;

    auto labelType = label.at(key_names::labelService);
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

std::vector<std::string> buildPosTagList(const Json& labelsArray)
{
  assert(labelsArray.is_array());
  std::map<size_t, std::string> posTagPositionMap;
  std::vector<std::string> posTagValues;
  auto lastTagPosition = posTagValues.size();

  for (const auto& label : labelsArray)
  {
    if (label.at(key_names::labelField) != "posTag")
      continue;

    size_t tagPosition = label.at("startToken");
    size_t tagEnd = label.at("endToken");
    if (tagEnd - tagPosition > 1)
      throw std::runtime_error("posTag endToken-startToken > 1");

    if (tagPosition > lastTagPosition)
      lastTagPosition = tagPosition;

    posTagPositionMap[tagPosition] = label.at("value");
  }

  if (lastTagPosition+1 > posTagPositionMap.size())
    throw std::runtime_error("There are missing posTag labels!");

  for (size_t i = 0; i <= lastTagPosition; ++i)
    posTagValues.push_back(posTagPositionMap[i]);

  return posTagValues;
}

}
