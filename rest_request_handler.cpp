#include "rest_request_handler.h"

#include <iomanip>
#include <sstream>

#include "nlohmann_json/json.hpp"

#include "label_processing.h"

using namespace Pistache;

using Json = nlohmann::json;

namespace
{

auto getContentType(const Http::Request& request)
{
  return request.headers().get<Http::Header::ContentType>()->mime();
}

}

void RestRequestHandler::onRequest(const Http::Request& request, Http::ResponseWriter response)
{
  std::cout << composeRequestDescription(request);

  if (!isRequestValid(request))
  {
    std::cout << "> Request Rejected\n";
    sendErrorResponse(request, response);
    return;
  }

  std::string lemmatizedJson;
  try
  {
    lemmatizedJson = lemmatizeRequestJson(request);
  }
  catch (const std::runtime_error& exception)
  {
    std::cout << "> Failed to process input JSON: " << exception.what() << "\n";
    response.send(Http::Code::Unprocessable_Entity, exception.what());
  }

  std::cout << "> Input JSON processed successfully, sending response...\n";

  response.setMime(Http::Mime::MediaType::fromString("application/json"));
  response.send(Http::Code::Ok, lemmatizedJson);

  std::cout << "> Done\n";
}

std::string RestRequestHandler::composeRequestDescription(const Http::Request& request) const
{
  std::stringstream description;
  description << "> Received Request\n";
  description << "  Host: " << request.address().host() << "\n";
  description << "  Port: " << request.address().port() << "\n";
  description << "  Method: " << request.method() << "\n";
  description << "  Resource: " << request.resource() << "\n";
  description << "  Content Type: " << getContentType(request).raw() << "\n";
  description << "  Body Length: " << request.body().size() << "\n";

  return description.str();
}

bool RestRequestHandler::isRequestValid(const Http::Request& request) const
{
  auto requestContentType = getContentType(request);
  const bool isPost = request.method() == Http::Method::Post;
  const bool isJson = requestContentType == Http::Mime::MediaType::fromString("application/json");
  return isPost && isJson;
}

void RestRequestHandler::sendErrorResponse(const Http::Request& request,
                                           Http::ResponseWriter& response) const
{
  if (request.method() != Http::Method::Post)
    response.send(Http::Code::Bad_Request, "Invalid request method; only POST is accepted.\n");

  if (getContentType(request) != Http::Mime::MediaType::fromString("application/json"))
    response.send(Http::Code::Unsupported_Media_Type,
                  "Invalid request content type; \"application/json\" expected.\n");
}

std::string RestRequestHandler::lemmatizeRequestJson(const Http::Request& request) const
{
  Json json = Json::parse(request.body());
  label_processing::findAndLemmatizeNerLabelsInJson(json);
  std::stringstream prettyOutputJson;
  prettyOutputJson << std::setw(2) << json << "\n";
  return prettyOutputJson.str();
}
