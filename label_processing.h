#ifndef LABEL_PROCESSING_H
#define LABEL_PROCESSING_H

#include <string>
#include <vector>

#include "nlohmann_json/json.hpp"

namespace key_names
{
const std::string docsKey = "docs";
const std::string labelsKey = "labels";
const std::string labelType = "serviceName";
}

namespace label_processing
{

std::vector<nlohmann::json> findNerLabels(const nlohmann::json& labelsArray);

nlohmann::json lemmatizeNerLabel(const nlohmann::json& nerLabel);

std::vector<nlohmann::json> lemmatizeNerLabels(const std::vector<nlohmann::json>& nerLabels);

void addLemmatizedLabels(nlohmann::json& targetLabelsArray,
                         const std::vector<nlohmann::json>& lemmatizedLabels);

}

#endif // LABEL_PROCESSING_H
