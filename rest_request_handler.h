#ifndef REST_REQUEST_HANDLER_H
#define REST_REQUEST_HANDLER_H

#include <pistache/endpoint.h>

class RestRequestHandler : public Pistache::Http::Handler
{
public:
  HTTP_PROTOTYPE(RestRequestHandler)

  void onRequest(const Pistache::Http::Request& request,
                 Pistache::Http::ResponseWriter response) override;
};

#endif // REST_REQUEST_HANDLER_H
