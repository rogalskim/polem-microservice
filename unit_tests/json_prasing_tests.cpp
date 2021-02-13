#define BOOST_TEST_MODULE json_parsing_tests

#include <iostream>

#include <boost/test/included/unit_test.hpp>

#include "../nlohmann_json/json.hpp"
#include "../label_processing.h"

using Json = nlohmann::json;
using namespace label_processing;


BOOST_AUTO_TEST_SUITE(ner_finding_tests)

BOOST_AUTO_TEST_CASE(given_lables_object__ner_label_finder_returns_correct_number_of_labels)
{
  auto testJson =
    R"({
      "labels":
       [
        {"serviceName": "NER"},
        {"serviceName": "NER"}
       ]
    })"_json;

  auto nerLabels = findNerLabels(testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL(nerLabels.size(), 2u);
}

BOOST_AUTO_TEST_CASE(ner_label_finder_ignores_labels_without_ner_type)
{
  auto testJson =
    R"({
      "labels":
       [
        {"serviceName": "tagger"},
        {"serviceName": "NER"},
        {"serviceName": "other_type"}
       ]
    })"_json;

  auto nerLabels = findNerLabels(testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL(nerLabels.size(), 1u);
}

BOOST_AUTO_TEST_CASE(ner_label_finder_ignores_labels_without_serviceName_key)
{
  auto testJson =
    R"({
      "labels":
       [
        {"different_key": "Hello!"},
        {}
       ]
    })"_json;

  auto nerLabels = findNerLabels(testJson.at(key_names::labelsKey));

  BOOST_TEST(nerLabels.empty());
}

BOOST_AUTO_TEST_CASE(ner_label_finder_returns_correct_labels)
{
  auto testNerLabel =
    R"({
        "end": 6,
        "endToken": 0,
        "fieldName": "namedEntityML",
        "name": "sys.Country",
        "score": 1.0,
        "serviceName": "NER",
        "start": 0,
        "startToken": 0,
        "value": "Polska"
      })"_json;

  auto testJson =
    R"({
      "labels":
       [
        {
          "end": 6,
          "endToken": 1,
          "fieldName": "lemmas",
          "name": "lemmas",
          "score": 1,
          "serviceName": "tagger",
          "start": 0,
          "startToken": 0,
          "value": [
            "polski"
          ]
        }
       ]
    })"_json;

  testJson.at(key_names::labelsKey).push_back(testNerLabel);

  auto nerLabels = findNerLabels(testJson.at(key_names::labelsKey));

  BOOST_REQUIRE_EQUAL(nerLabels.size(), 1u);
  BOOST_TEST(nerLabels[0] == testNerLabel, "Returned label is not equal to expected label");
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(posTag_list_builder_tests)

BOOST_AUTO_TEST_CASE(buildPosTagList_returns_vector_of_Jsons_of_size_equal_to_posTag_label_count)
{
  auto testJson =
    R"({
      "labels":
       [
        {
          "startToken": 0,
          "endToken": 1,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "adj:sg:nom:f:pos"
        },
        {
          "startToken": 1,
          "endToken": 2,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "spam:spam:eggs:spam"
        },
        {"fieldName": "lemmas"},
        {"fieldName": "NER"}
       ]
    })"_json;

  std::vector<Json> posTagList
      = label_processing::buildPosTagList(testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL(posTagList.size(), 2u);
}

BOOST_AUTO_TEST_CASE(buildPosTagList_throws_if_there_is_a_posTag_label_missing)
{
  auto testJson =
    R"({
      "labels":
       [
        {
          "startToken": 0,
          "endToken": 1,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "FIRST TAG"
        },
        {
          "startToken": 1,
          "endToken": 2,
          "fieldName": ">> NOT POSTAG <<",
          "serviceName": "EGGS",
          "value": "SECOND TAG"
        },
        {
          "startToken": 2,
          "endToken": 3,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "THIRD TAG"
        }
       ]
    })"_json;

  BOOST_CHECK_THROW(label_processing::buildPosTagList(testJson.at(key_names::labelsKey)),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(buildPosTagList_expects_exactly_1_posTag_per_token_otherwise_thorws)
{
  auto testJson =
    R"({
      "labels":
       [
        {
          "startToken": 0,
          "endToken": 3,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "endToken - startToken > 1"
        }
       ]
    })"_json;

  BOOST_CHECK_THROW(label_processing::buildPosTagList(testJson.at(key_names::labelsKey)),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(buildPosTagList_returns_list_of_found_posTag_labels)
{
  std::vector<Json> expectedPosTagList =
  {
    R"({
          "startToken": 0,
          "endToken": 1,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "FIRST TAG"
      })"_json,
    R"({
          "startToken": 1,
          "endToken": 2,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "SECOND TAG"
      })"_json,
    R"({
          "startToken": 2,
          "endToken": 3,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "THIRD TAG"
      })"_json
  };

  auto testJson =R"({"labels": []})"_json;
  for (const auto& testPosTag : expectedPosTagList)
    testJson.at("labels").push_back(testPosTag);

  auto outputPosTagList = label_processing::buildPosTagList(testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL_COLLECTIONS(expectedPosTagList.begin(), expectedPosTagList.end(),
                                outputPosTagList.begin(), outputPosTagList.end());
}

BOOST_AUTO_TEST_CASE(buildPosTagList_returns_posTag_list_sorted_by_startToken)
{
  std::vector<Json> expectedPosTagList =
  {
    R"({
          "startToken": 0,
          "endToken": 1,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "FIRST TAG"
      })"_json,
    R"({
          "startToken": 1,
          "endToken": 2,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "SECOND TAG"
      })"_json,
    R"({
          "startToken": 2,
          "endToken": 3,
          "fieldName": "posTag",
          "serviceName": "tagger",
          "value": "THIRD TAG"
      })"_json
  };

  auto testJson =R"({"labels": []})"_json;
  testJson.at(key_names::labelsKey).push_back(expectedPosTagList[1]);
  testJson.at(key_names::labelsKey).push_back(expectedPosTagList[2]);
  testJson.at(key_names::labelsKey).push_back(expectedPosTagList[0]);

  auto outputPosTagList = label_processing::buildPosTagList(testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL_COLLECTIONS(expectedPosTagList.begin(), expectedPosTagList.end(),
                                outputPosTagList.begin(), outputPosTagList.end());
}

BOOST_AUTO_TEST_SUITE_END()




BOOST_AUTO_TEST_CASE(lemmatizer_lammatizes)
{
  auto input = R"({"value": "Alejach Jerozolimskich"})"_json;
  auto expected = R"({"value": "aleje jerozolimskie"})"_json;

  auto output = label_processing::lemmatizeNerLabel(input);

  BOOST_TEST(output == expected);
}
