# micro_router

Tiny HTTP router for lightweight C++ servers.

`micro_router` provides fast and minimal HTTP route matching with named
parameter extraction. It is designed for small services, embedded
servers, edge runtimes and custom frameworks.

Header-only. Zero heavy dependencies. Clean API.

## Why micro_router?

Unlike large web frameworks, this library:

-   Provides simple method + path routing
-   Supports named parameters (`:id`, `{id}`)
-   Extracts route params automatically
-   Ignores query strings during matching
-   Tolerates trailing slashes
-   Is fully header-only
-   Has no external dependencies

Perfect for:

-   Custom HTTP servers
-   Embedded services
-   Lightweight REST APIs
-   Edge runtimes
-   Prototyping frameworks
-   Learning HTTP routing internals

## Installation

### Using Vix Registry

``` bash
vix add gaspardkirira/micro_router
vix deps
```

### Manual

Clone the repository:

``` bash
git clone https://github.com/GaspardKirira/micro_router.git
```

Add the `include/` directory to your project.

## Quick Static Route Example

``` cpp
#include <micro_router/micro_router.hpp>
#include <iostream>

int main()
{
  using namespace micro_router;

  Router router;

  router.get("/health", [](const Request&, Response& res) {
    res.body = "ok";
  });

  Request req{Method::Get, "/health"};
  Response res;

  if (router.dispatch(req, res))
    std::cout << res.body << "\n";
}
```

## Quick Parameter Example

``` cpp
#include <micro_router/micro_router.hpp>
#include <iostream>

int main()
{
  using namespace micro_router;

  Router router;

  router.get("/users/:id", [](const Request& req, Response& res) {
    res.body = "User id = " + req.params.at("id");
  });

  Request req{Method::Get, "/users/42"};
  Response res;

  router.dispatch(req, res);

  std::cout << res.body << "\n";
}
```

## Supported Route Patterns

### Static
```
    /health
    /status
```
### Colon parameters
```
    /users/:id
    /posts/:postId/comments/:id
```
### Braced parameters
```
    /users/{id}
    /posts/{postId}/comments/{id}
```
Both styles are supported.

## Features

-   Header-only
-   C++17 compatible
-   Fast segment-based matching
-   Named parameter extraction
-   Multiple HTTP methods
-   Optional "Any" method
-   Query string ignored during match
-   Trailing slash tolerant
-   Minimal memory overhead

## API Overview

``` cpp
micro_router::Router router;

router.get("/health", handler);
router.post("/items", handler);
router.any("/debug", handler);

micro_router::Request req{micro_router::Method::Get, "/users/42"};
micro_router::Response res;

if (router.dispatch(req, res))
{
  // route matched
}
```

## Design Philosophy

micro_router focuses on:

-   Simplicity over abstraction layers
-   Explicit method registration
-   No magic
-   Minimal runtime overhead
-   Easy integration into existing servers

It does not provide:

-   HTTP parsing
-   Middleware stacks
-   Serialization
-   Thread pools

It is intentionally small and composable.

## Tests

Run:

``` bash
vix build
vix tests
```

## License

MIT License
Copyright (c) Gaspard Kirira

