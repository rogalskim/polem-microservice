#include <cassert>
#include <cctype>
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

Json lemmatizeNerLabel(const Json& nerLabel,
                       const std::string& posTags,
                       const std::string& lemmaTags,
                       CascadeLemmatizer& lemmatizer)
{
  assert(nerLabel.is_object());
  assert(nerLabel.contains("value"));

  const std::string& inputValue = nerLabel["value"];

  auto output = lemmatizer.lemmatize(inputValue.c_str(), lemmaTags.c_str(), posTags.c_str(), false);

  Json lemmatizedNer = nerLabel;
  std::string strOutput;
  output.toUTF8String(strOutput);
  lemmatizedNer["value"] = strOutput;
  lemmatizedNer[key_names::labelField] = "polem";
  lemmatizedNer["name"] = "polem";
  lemmatizedNer[key_names::labelService] = "Polem";

  return lemmatizedNer;
}

std::string toLowercase(std::string str)
{
  std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c){return std::tolower(c);});
  return str;
}

std::tuple<std::string, std::string>
buildPosAndLemmaStringsForNerLabel(const Json& nerLabel,
                                   const std::vector<std::string>& posTagValues,
                                   const std::vector<std::string>& lemmaTagValues)
{
  const int nerStartToken = nerLabel.at("startToken");
  const int nerEndToken = nerLabel.at("endToken");
  if (int(posTagValues.size()) <= nerEndToken)
      throw std::runtime_error("Missing posTag and/or lemma labels!");

  std::string posTags, lemmaTags;
  for (int token = nerStartToken; token <= nerEndToken; ++token)
  {
    posTags += toLowercase(posTagValues[token]);
    lemmaTags += toLowercase(lemmaTagValues[token]);

    if (token == nerEndToken)
      continue;

    posTags += " ";
    lemmaTags += " ";
  }

  return std::make_tuple(posTags, lemmaTags);
}

std::vector<Json> lemmatizeNerLabels(const std::vector<nlohmann::json>& nerLabels,
                                     const std::vector<std::string>& posTagValues,
                                     const std::vector<std::string>& lemmaTagValues,
                                     CascadeLemmatizer& lemmatizer)
{ 
  if (posTagValues.size() != lemmaTagValues.size())
    throw std::runtime_error("Different counts of posTag and lemma labels!");

  std::vector<Json> lemmatizedLabels;
  for (const auto& nerLabel : nerLabels)
  {
    auto posTagAndLemma = buildPosAndLemmaStringsForNerLabel(nerLabel, posTagValues, lemmaTagValues);
    lemmatizedLabels.push_back(lemmatizeNerLabel(nerLabel,
                                                 std::get<0>(posTagAndLemma),
                                                 std::get<1>(posTagAndLemma),
                                                 lemmatizer));
  }
  return lemmatizedLabels;
}

void addLemmatizedLabels(Json& targetLabelsArray, const std::vector<Json>& lemmatizedLabels)
{
  assert(targetLabelsArray.is_array());
  for (const auto& lemmatizedLabel : lemmatizedLabels)
    targetLabelsArray.push_back(lemmatizedLabel);
}

std::vector<std::string> buildTagValueList(const std::string& tagFieldName,
                                           const nlohmann::json& labelsArray)
{
  assert(labelsArray.is_array());
  std::map<size_t, std::string> tagPositionMap;
  std::vector<std::string> tagValues;
  auto lastTagPosition = tagValues.size();

  for (const auto& label : labelsArray)
  {
    if (label.at(key_names::labelField) != tagFieldName)
      continue;

    size_t tagPosition = label.at("startToken");
    size_t tagEnd = label.at("endToken");
    if (tagEnd - tagPosition != 1)
      throw std::runtime_error("posTag endToken-startToken != 1");

    if (tagPosition > lastTagPosition)
      lastTagPosition = tagPosition;

    if (label.at("value").is_array())
      tagPositionMap[tagPosition] = label.at("value")[0];
    else
      tagPositionMap[tagPosition] = label.at("value");
  }

  if (lastTagPosition+1 > tagPositionMap.size())
    throw std::runtime_error("There are missing posTag labels!");

  for (size_t i = 0; i <= lastTagPosition; ++i)
    tagValues.push_back(tagPositionMap[i]);

  return tagValues;
}

void findAndLemmatizeNerLabelsInJson(nlohmann::json& targetJson)
{
  if (!targetJson.contains(key_names::docsKey))
    throw std::runtime_error("Input JSON doesn't contain \"" + key_names::docsKey + "\" key");

  Json& docs = targetJson.at(key_names::docsKey);
  assert(docs.is_array());

  if (docs.empty())
    throw std::runtime_error("\"" + key_names::docsKey + "\" item is empty");

  CascadeLemmatizer lemmatizer = CascadeLemmatizer::assembleLemmatizer();

  for (auto& [key, doc] : docs.items())
  {
    if (!doc.is_object() || !doc.contains(key_names::labelsKey))
      continue;

    Json& labelArray = doc.at(key_names::labelsKey);
    if (!labelArray.is_array() || labelArray.empty())
      continue;

    try
    {
      const auto& nerLabels = label_processing::findNerLabels(labelArray);
      const auto& posTagValues = label_processing::buildTagValueList("posTag", labelArray);
      const auto& lemmaTagValues = label_processing::buildTagValueList("lemmas", labelArray);
      const auto&  lemmatizedLabels = label_processing::lemmatizeNerLabels(nerLabels,
                                                                           posTagValues,
                                                                           lemmaTagValues,
                                                                           lemmatizer);
      label_processing::addLemmatizedLabels(labelArray, lemmatizedLabels);
    }
    catch (const std::runtime_error& exception)
    {
      std::cout << std::string("Processing a doc element failed!\n") + exception.what() + "\n";
    }
  }
}

}
