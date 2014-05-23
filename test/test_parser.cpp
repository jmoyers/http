#include "gtest/gtest.h"
#include "../parser.h"
#include  "../headers.h"

using namespace Multiplexer;

TEST(ParserTests, Basic)
{
  const char *request = 
    "GET / HTTP/1.0\r\n"
    "Host: localhost:8080\r\n"
    "User-Agent: ApacheBench/2.3\r\n"
    "Accept: */*\r\n\r\n\0";

  Parser p(request, strlen(request));
  p.run();

  auto h = p.getHeaders();

  EXPECT_EQ(h->getMethod(), Headers::Method::GET);
  EXPECT_EQ(h->getField("Host"), "localhost:8080");
  EXPECT_EQ(h->getField("User-Agent"), "ApacheBench/2.3");
  EXPECT_EQ(h->getField("Accept"), "*/*");
  EXPECT_EQ(h->getUpgrade(), Headers::Upgrade::NOUPGRADE);
}

TEST(ParserTests, WebSockets)
{
  const char *request = 
    "GET /chat HTTP/1.1\r\n"
    "Upgrade: websocket\r\n"
    "Connection: Upgrade\r\n"
    "Host: localhost:8080\r\n"
    "Origin: http://localhost:8888\r\n"
    "Pragma: no-cache\r\n"
    "Cache-Control: no-cache\r\n"
    "Sec-WebSocket-Key: /oCIRrmB40qjf/mVxZfceA==\r\n"
    "Sec-WebSocket-Version: 13\r\n"
    "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits, "
    "x-webkit-deflate-frame\r\n"
    "User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_3) AppleWebKit"
    "/537.36 (KHTML, like Gecko) Chrome/35.0.1916.114 Safari/537.36\r\n\0";

  Parser p(request, strlen(request));
  p.run();

  auto h = p.getHeaders();

  EXPECT_EQ(h->getMethod(), Headers::Method::GET);
  EXPECT_EQ(h->getField("Upgrade"), "websocket");
  EXPECT_EQ(h->getField("Connection"), "Upgrade");
  EXPECT_EQ(h->getUpgrade(), Headers::Upgrade::WEBSOCKET);
  EXPECT_EQ(h->getField("Host"), "localhost:8080");
  EXPECT_EQ(h->getField("Origin"), "http://localhost:8888");
  EXPECT_EQ(h->getField("Pragma"), "no-cache");
  EXPECT_EQ(h->getField("Cache-Control"), "no-cache");
  EXPECT_EQ(h->getField("Sec-WebSocket-Key"), "/oCIRrmB40qjf/mVxZfceA==");
  EXPECT_EQ(h->getField("Sec-WebSocket-Version"), "13");
  EXPECT_EQ(h->getField("Sec-WebSocket-Extensions"), 
      "permessage-deflate; client_max_window_bits, x-webkit-deflate-frame");
  EXPECT_EQ(h->getField("User-Agent"), 
      "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_9_3) AppleWebKit"
      "/537.36 (KHTML, like Gecko) Chrome/35.0.1916.114 Safari/537.36");
}
