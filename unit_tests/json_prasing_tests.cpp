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


BOOST_AUTO_TEST_SUITE(tag_value_list_builder_tests)

BOOST_AUTO_TEST_CASE(buildTagValueList_returns_vector_of_strings_of_size_equal_to_posTag_label_count)
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

  std::vector<std::string> posTagValueList
      = label_processing::buildTagValueList("posTag", testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL(posTagValueList.size(), 2u);
}

BOOST_AUTO_TEST_CASE(buildTagValueList_throws_if_there_is_a_posTag_label_missing)
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

  BOOST_CHECK_THROW(label_processing::buildTagValueList("posTag", testJson.at(key_names::labelsKey)),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(buildTagValueList_expects_exactly_1_posTag_per_token_otherwise_thorws)
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

  BOOST_CHECK_THROW(label_processing::buildTagValueList("posTag", testJson.at(key_names::labelsKey)),
                    std::runtime_error);
}

BOOST_AUTO_TEST_CASE(buildTagValueList_returns_list_of_found_posTag_values)
{
  std::vector<Json> posTagList =
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
  std::vector<std::string> expectedPosTagValues;
  auto testJson =R"({"labels": []})"_json;
  for (const auto& testPosTag : posTagList)
  {
    expectedPosTagValues.push_back(testPosTag.at("value"));
    testJson.at("labels").push_back(testPosTag);
  }

  auto outputPosTagValues = label_processing::buildTagValueList("posTag",
                                                                testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL_COLLECTIONS(expectedPosTagValues.begin(), expectedPosTagValues.end(),
                                outputPosTagValues.begin(), outputPosTagValues.end());
}

BOOST_AUTO_TEST_CASE(buildTagValueList_returns_posTag_list_sorted_by_startToken)
{
  std::vector<Json> posTagList =
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

  std::vector<std::string> expectedPosTagValues;
  for (const auto& testPosTag : posTagList)
    expectedPosTagValues.push_back(testPosTag.at("value"));

  auto testJson =R"({"labels": []})"_json;
  testJson.at(key_names::labelsKey).push_back(posTagList[1]);
  testJson.at(key_names::labelsKey).push_back(posTagList[2]);
  testJson.at(key_names::labelsKey).push_back(posTagList[0]);

  auto outputPosTagList = label_processing::buildTagValueList("posTag",
                                                              testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL_COLLECTIONS(expectedPosTagValues.begin(), expectedPosTagValues.end(),
                                outputPosTagList.begin(), outputPosTagList.end());
}

BOOST_AUTO_TEST_CASE(buildTagValueList_can_build_both_posTag_and_lemmas_lists)
{
  std::vector<Json> posTagList =
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
      })"_json
  };

  std::vector<Json> lemmaTagList =
  {
    R"({
          "startToken": 0,
          "endToken": 1,
          "fieldName": "lemmas",
          "serviceName": "tagger",
          "value": ["FIRST LEMMA"]
      })"_json,
    R"({
          "startToken": 1,
          "endToken": 2,
          "fieldName": "lemmas",
          "serviceName": "tagger",
          "value": ["SECOND LEMMA"]
      })"_json
  };

  std::vector<std::string> expectedPosTagValues;
  auto testJson =R"({"labels": []})"_json;
  for (const auto& testPosTag : posTagList)
  {
    expectedPosTagValues.push_back(testPosTag.at("value"));
    testJson.at("labels").push_back(testPosTag);
  }
  std::vector<std::string> expectedLemmaValues;
  for (const auto& testLemma : lemmaTagList)
  {
    expectedLemmaValues.push_back(testLemma.at("value")[0]);
    testJson.at("labels").push_back(testLemma);
  }

  auto outputPosTagValues
      = label_processing::buildTagValueList("posTag", testJson.at(key_names::labelsKey));
  auto outputLemmaTagValues
      = label_processing::buildTagValueList("lemmas", testJson.at(key_names::labelsKey));

  BOOST_CHECK_EQUAL_COLLECTIONS(expectedPosTagValues.begin(), expectedPosTagValues.end(),
                                outputPosTagValues.begin(), outputPosTagValues.end());
  BOOST_CHECK_EQUAL_COLLECTIONS(expectedLemmaValues.begin(), expectedLemmaValues.end(),
                                outputLemmaTagValues.begin(), outputLemmaTagValues.end());
}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(lemmatization_tests)

BOOST_AUTO_TEST_CASE(lemmatizeNerLabels_throws_if_cant_find_posTag_needed_for_a_NER_label)
{
  auto testJson =
    R"({
      "labels":
       [
        {
          "end": 25,
          "endToken": 2,
          "fieldName": "namedEntityML",
          "name": "sys.Localization",
          "score": 1.0,
          "serviceName": "NER",
          "start": 3,
          "startToken": 1,
          "value": "Alejach Jerozolimskich"
        }
       ]
    })"_json;
}

BOOST_AUTO_TEST_CASE(lemmatizer_lammatizes)
{
  auto input = R"({"value": "Alejach Jerozolimskich"})"_json;
  auto expected = R"({"value": "aleje jerozolimskie"})"_json;

  auto output = label_processing::lemmatizeNerLabel(input);

  BOOST_TEST(output == expected);
}

BOOST_AUTO_TEST_SUITE_END()
