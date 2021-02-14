#ifndef REST_REQUEST_HANDLER_H
#define REST_REQUEST_HANDLER_H

#include <string>

#include <pistache/endpoint.h>


class RestRequestHandler : public Pistache::Http::Handler
{
public:
  HTTP_PROTOTYPE(RestRequestHandler)

  void onRequest(const Pistache::Http::Request& request,
                 Pistache::Http::ResponseWriter response) override;

private:
  std::string composeRequestDescription(const Pistache::Http::Request& request) const;
  bool isRequestValid(const Pistache::Http::Request& request) const;
  void sendErrorResponse(const Pistache::Http::Request& request,
                         Pistache::Http::ResponseWriter& response) const;
  std::string lemmatizeRequestJson(const Pistache::Http::Request& request) const;
};

#endif // REST_REQUEST_HANDLER_H
