#include "bandit/bandit.h"
#include "socket.h"
#include <iostream>
#include <string>

using namespace bandit;
using namespace Net;
using namespace std;

go_bandit([]()
{
  describe("Socket", []()
  {
    it("should let you construct with no fd and configure to acquire", []
    {
      Socket s;
      s.configure();
      AssertThat(s.fd(), !Equals(0));
      AssertThat(s.err(), Equals(0));
      AssertThat(s.state(), Equals(Socket::INITIALIZED));
    });

    it("should let you configure to listen on an address and port", []
    {
      Socket s;
      s.configure("127.0.0.1", 8080);
      AssertThat(s.fd(), !Equals(0));
      AssertThat(s.err(), Equals(0));
      AssertThat(s.state(), Equals(Socket::INITIALIZED));
      
      s.bind();

      AssertThat(s.fd(), !Equals(0));
      AssertThat(s.err(), Equals(0));
      AssertThat(s.state(), Equals(Socket::BOUND));

      s.listen();
      
      AssertThat(s.fd(), !Equals(0));
      AssertThat(s.err(), Equals(0));
      AssertThat(s.state(), Equals(Socket::LISTENING));
    });

    it("should fail to acquire socket with priveleged port number", []
    {
      Socket s;
      s.configure("127.0.0.1", 80);
      s.bind();
      AssertThat(s.err(), IsLessThan(0));
    });

    it("should allow you to send and receive data", []
    {
      Socket server;
      Socket client;

      server.configure("127.0.0.1", 8080);
      server.bind();
      server.listen();

      AssertThat(server.state(), Equals(Socket::LISTENING));

      client.configure();
      client.connect("127.0.0.1", 8080);
      AssertThat(client.state(), Equals(Socket::CONNECTED));

      char send_buf[25] = "This is sent from client";
      char recv_buf[25];

      int sent = client.send(send_buf, strlen(send_buf));
      Socket connection(server.accept());
      int received = connection.recv(recv_buf, 25);

      // bytes_sent, bytes_received
      AssertThat(client.err(), IsGreaterThan(0));
      AssertThat(server.err(), IsGreaterThan(0));

      AssertThat(recv_buf, Equals("This is sent from client"));
      AssertThat(received, Equals(sent));
    });

    it("should default to a blocking socket on construct", []
    {
      Socket s;
      AssertThat(s.type(), Equals(Socket::BLOCKING));
    });

    it("should allow you to construct a non-blocking socket", []
    {
      Socket s(Socket::NONBLOCKING);
      AssertThat(s.type(), Equals(Socket::NONBLOCKING));
    });

    it("should have connecting state on non-blocking socket who connects", []
    {
      Socket server;

      server.configure("127.0.0.1", 8080);
      server.bind();
      server.listen();
      AssertThat(server.state(), Equals(Socket::LISTENING));

      Socket client(Socket::NONBLOCKING);
      client.configure();
      client.connect("127.0.0.1", 8080);

      AssertThat(client.state(), Equals(Socket::CONNECTING));
      // later, you can use epoll or kqueue to check whether connect succeeded
    });

    it("should go into an invalid state when trying to connect to bad addr", []
    {
      Socket client;
      client.configure();
      client.connect("127.0.0.1", 8080);
      AssertThat(client.state(), Equals(Socket::INVALID));
    });
  });
});

int main(int argc, char **argv)
{
  return run(argc, argv);
}
