#include "bandit/bandit.h"
#include "parser.h"
#include <iostream>

using namespace bandit;
using namespace Http;
using namespace std;

go_bandit([]()
{
  describe("Parser", []()
  {
    it("should survive a 0 length buffer", []
    {
      const char request[] = "";
      Parser parser(request, strlen(request));
      auto state = parser.parse();

      AssertThat(state, Equals(Parser::State::DONE));
    });

    it("should parse a GET request", []
    {
      const char request[] = 
        "GET / HTTP/1.0";
      Parser parser(request, strlen(request));
      auto state = parser.parse();
      auto headers = parser.get_headers();
      auto version = headers->get_http_version();

      AssertThat(state, Equals(Parser::State::DONE));
      AssertThat(parser.get_headers()->get_method(), 
          Equals(Headers::Method::GET));
    });

    it("should parse a HEAD request", []
    {
      const char request[] = 
        "HEAD /testpath HTTP/1.1";

      Parser parser(request, strlen(request));
      auto state = parser.parse();
      auto headers = parser.get_headers();

      AssertThat(state, Equals(Parser::State::DONE));
      AssertThat(headers->get_method(), Equals(Headers::Method::HEAD));
    });

    it("should reject incomplete http verb", []
    {
      const char request[] = 
        "GE.... thing thats not a verb";

      Parser parser(request, strlen(request));
      auto state = parser.parse();

      AssertThat(state, Equals(Parser::State::BROKEN));
      AssertThat(parser.get_headers()->get_method(), 
          Equals(Headers::Method::NONE));
    });

    it("should parse a basic get request", []
    {
      const char request[] = 
        "GET / HTTP/1.0\r\n"
        "Host: localhost:8080\r\n"
        "User-Agent: ApacheBench/2.3\r\n"
        "Accept: */*\r\n\r\n\0"; 

      Parser p(request, strlen(request));
      p.parse();

      auto h = p.get_headers();

      AssertThat(h->get_method(), Equals(Headers::Method::GET));
      AssertThat(h->get_field("host"), Equals("localhost:8080"));
      AssertThat(h->get_field("user-agent"), Equals("ApacheBench/2.3"));
      AssertThat(h->get_field("accept"), Equals("*/*"));
      AssertThat(h->get_upgrade(), Equals(Headers::Upgrade::NONE));
    });

    it("should be able to parse a websocket GET request", []
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

      auto h = p.get_headers();

      AssertThat(h->get_method(), Equals(Headers::Method::GET));
      AssertThat(h->get_field("upgrade"), Equals("websocket"));
      AssertThat(h->get_field("connection"), Equals("Upgrade"));
      AssertThat(h->get_upgrade(), Equals(Headers::Upgrade::WEBSOCKET));
      AssertThat(h->get_field("sec-websocket-key"), 
          Equals("/oCIRrmB40qjf/mVxZfceA=="));
      AssertThat(h->get_field("sec-websocket-version"), Equals("13"));
      AssertThat(h->get_field("sec-websocket-extensions"), 
          Equals("permessage-deflate; client_max_window_bits, "
            "x-webkit-deflate-frame"));
    });
  });
});

int main(int argc, char **argv)
{
  return run(argc, argv);
}
