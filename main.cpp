#include <cassert>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <pistache/endpoint.h>
#include <pistache/net.h>

#include "label_processing.h"

#include "nlohmann_json/json.hpp"

using Json = nlohmann::json;
namespace fs = std::filesystem;

Json readJsonFromDisk(const fs::path& path)
{
  if (!fs::exists(path))
    throw std::invalid_argument("File doesn't exist");
  if (path.extension() != ".json")
    throw std::invalid_argument("File is not of .json type");

  std::ifstream jsonFile(path, std::ifstream::in);

  if (!jsonFile.good())
    throw std::runtime_error("Failed to open file " + path.string());

  Json json;
  jsonFile >> json;

  jsonFile.close();

  return json;
}

using namespace Pistache;

class RestRequestHandler : public Http::Handler
{
public:
  HTTP_PROTOTYPE(RestRequestHandler)

  void onRequest(const Http::Request& request, Http::ResponseWriter response) override
  {
    if (request.method() != Http::Method::Post)
      response.send(Http::Code::Bad_Request, "Invalid request method; only POST is accepted.\n");

    auto requestContentHeader = request.headers().get<Http::Header::ContentType>();
    auto requestContentType = requestContentHeader->mime();
    if (requestContentType != Http::Mime::MediaType::fromString("application/json"))
      response.send(Http::Code::Bad_Request, "Invalid request content type; \"application/json\" expected.\n");

    std::stringstream reply;
    reply << "Received Request\n";
    reply << "----------------\n";
    reply << "Host: " << request.address().host() << "\n";
    reply << "Port: " << request.address().port() << "\n";
    reply << "Method: " << request.method() << "\n";
    reply << "Resource: " << request.resource() << "\n";
    reply << "Header Count: " << request.headers().list().size() << "\n";
    reply << "Content Type: " << requestContentType.raw() << "\n";
    reply << "Body: \n" << request.body() << "\n\n";

    response.send(Http::Code::Ok, reply.str() + "\n");
  }
};

int main()
{
  std::cout << "Starting the server!\n";

  Address address(Ipv4::any(), Port(5000));

  const int serverThreadCount = 1;
  auto options = Http::Endpoint::options().threads(serverThreadCount);
  Http::Endpoint server(address);

  server.init(options);
  server.setHandler(Http::make_handler<RestRequestHandler>());
  server.serve();

  return 0;
/*
  try
  {
    auto json = readJsonFromDisk("../data/test_input.json");
    label_processing::findAndLemmatizeNerLabelsInJson(json);
    std::cout << std::setw(4) << json << std::endl;
    return 0;
  }
  catch (const std::runtime_error& exception)
  {
    std::cout << exception.what() << "\nAborting...\n";
    return 1;
  }
*/
}
