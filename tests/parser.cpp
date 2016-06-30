#include "bandit/bandit.h"
#include "parser.h"
#include "headers.h"

using namespace bandit;
using namespace Http;

go_bandit([]()
{
  describe("Parser", []()
  {
    it("should parse a basic get request", []()
    {
      const char request[] = 
        "GET / HTTP/1.0\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: ApacheBench/2.3\r\n"
        "Accept: */*\r\n\r\n\0";

      Parser p(request, strlen(request));
      p.parse();

      auto h = p.getHeaders();

      AssertThat(h->getMethod(), Equals(Headers::Method::GET));
      AssertThat(h->getField("host"), Equals("localhost:8080"));
      AssertThat(h->getField("user-agent"), Equals("ApacheBench/2.3"));
      AssertThat(h->getField("accept"), Equals("*/*"));
      AssertThat(h->getUpgrade(), Equals(Headers::Upgrade::NONE));
    });

    it("should be able to parse a websocket GET request", []()
    {
      const char request[] = 
        "GET /chat HTTP/1.1\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: /oCIRrmB40qjf/mVxZfceA==\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Sec-WebSocket-Extensions: permessage-deflate; client_max_window_bits, "
        "x-webkit-deflate-frame\r\n";

      Parser p(request, strlen(request));
      p.parse();

      auto h = p.getHeaders();

      AssertThat(h->getMethod(), Equals(Headers::Method::GET));
      AssertThat(h->getField("upgrade"), Equals("websocket"));
      AssertThat(h->getField("connection"), Equals("Upgrade"));
      AssertThat(h->getUpgrade(), Equals(Headers::Upgrade::WEBSOCKET));
      AssertThat(h->getField("sec-websocket-key"), 
          Equals("/oCIRrmB40qjf/mVxZfceA=="));
      AssertThat(h->getField("sec-websocket-version"), Equals("13"));
      AssertThat(h->getField("sec-websocket-extensions"), 
          Equals("permessage-deflate; client_max_window_bits, "
            "x-webkit-deflate-frame"));
    });
  });
});

int main(int argc, char **argv)
{
  return run(argc, argv);
}
