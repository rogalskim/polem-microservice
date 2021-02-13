#ifndef LABEL_PROCESSING_H
#define LABEL_PROCESSING_H

#include <string>
#include <tuple>
#include <vector>

#include "nlohmann_json/json.hpp"

namespace key_names
{
const std::string docsKey = "docs";
const std::string labelsKey = "labels";
const std::string labelService = "serviceName";
const std::string labelField = "fieldName";
}

namespace label_processing
{

std::vector<nlohmann::json> findNerLabels(const nlohmann::json& labelsArray);

std::vector<std::string> buildTagValueList(const std::string& tagFieldName,
                                           const nlohmann::json& labelsArray);

nlohmann::json lemmatizeNerLabel(const nlohmann::json& nerLabel,
                                 const std::string& posTags,
                                 const std::string& lemmaTags);

std::tuple<std::string, std::string>
buildPosAndLemmaStringsForNerLabel(const nlohmann::json& nerLabel,
                                   const std::vector<std::string>& posTagValues,
                                   const std::vector<std::string>& lemmaTagValues);

std::vector<nlohmann::json> lemmatizeNerLabels(const std::vector<nlohmann::json>& nerLabels,
                                               const std::vector<std::string>& posTagValues,
                                               const std::vector<std::string>& lemmaTagValues);

void addLemmatizedLabels(nlohmann::json& targetLabelsArray,
                         const std::vector<nlohmann::json>& lemmatizedLabels);

void findAndLemmatizeNerLabelsInJson(nlohmann::json& targetJson);

}

#endif // LABEL_PROCESSING_H
