#ifndef __SOCKET_H
#define __SOCKET_H

#include <netinet/in.h> // sockaddr_in
#include <arpa/inet.h>  // inet_addr
#include <string.h>     // strerror
#include <errno.h>      // errno, set by syscalls
#include <fcntl.h>      // fcntl, F_SETFL, O_NONBLOCK
#include <sys/socket.h> // bind, listen, accept, connect
#include <unistd.h>     // close, read, write

#include "log.h"

namespace Net
{

class Socket
{
  public:
    enum State {
      INVALID,
      INITIALIZED,
      BOUND,
      LISTENING,
      ACCEPTED,   // a new client socket from listening socket
      CONNECTING, // nonblocking socket when calling connect()
      CONNECTED,
      CLOSED
    };

    enum Type {
      BLOCKING,
      NONBLOCKING
    };

    typedef uintptr_t FD;
    typedef struct sockaddr_in IPV4;

    Socket(FD fd, State state = INVALID, Type type = BLOCKING);
    Socket(Type type = BLOCKING);
    ~Socket() { close(); }

    int configure(const char *, int); 
    int configure(); 
    int bind();
    int listen();
    int accept();
    int close();

    int connect(const char *, int);
    int send(const char *, size_t);
    int recv(char *, size_t);

    State state()           { return m_state; }
    Type type()             { return m_type; }
    int err()               { return m_err; }
    FD fd()                 { return m_fd; }

    static void ipv4(IPV4&, const char *, int&);
  private:
    State m_state   = INVALID;
    FD m_fd       = 0;
    FD m_accepted = 0;
    int m_reuse   = 1;
    int m_backlog = 1000;
    Type m_type   = BLOCKING;
    int m_err     = -1;

    IPV4 m_listen_addr;
    IPV4 m_send_addr;

    static const int RECEIVE_SIZE = 1024;
    char m_buf[RECEIVE_SIZE];
}; // class

}  // namespace

#endif
