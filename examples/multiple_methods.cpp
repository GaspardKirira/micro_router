#include <micro_router/micro_router.hpp>
#include <iostream>

int main()
{
  using namespace micro_router;

  Router router;

  router.get("/items", [](const Request &, Response &res)
             { res.body = "List items"; });

  router.post("/items", [](const Request &, Response &res)
              {
    res.status = 201;
    res.body = "Item created"; });

  {
    Request req{Method::Get, "/items"};
    Response res;
    router.dispatch(req, res);
    std::cout << "GET -> " << res.body << "\n";
  }

  {
    Request req{Method::Post, "/items"};
    Response res;
    router.dispatch(req, res);
    std::cout << "POST -> " << res.body << "\n";
  }

  return 0;
}
