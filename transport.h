#ifndef __TRANSPORT_H
#define __TRANSPORT_H

#include <netinet/in.h> // sockaddr_in
#include <sys/event.h>  // kevent
#include <string.h>     // strerror
#include <string>       // std::string
#include <inttypes.h>   // PRIXPTR (printing pointers)
#include <arpa/inet.h>  // inet_addr
#include <errno.h>      // errno, set by syscalls
#include <fcntl.h>      // fcntl, F_SETFL, O_NONBLOCK
#include <sys/socket.h> // bind, listen, accept, connect
#include <unistd.h>     // close, read, write

#include "Log.h"

namespace Net
{

/**
 * Server transport state machine
 * 1. Acquire socket with socket(), set fd to nonblocking with fcntl()
 * 2. Bind socket to address/port with bind()
 * 3. Begin listening with listen()
 * 4. Subscribe to kqueue with kqueue(), EV_SET(), kevent()
 * 5. Loop for kqueue events with k_event()
 *
 * Client transport state machine
 * TODO
 */
class Transport {
  protected:
    enum SocketState {
      INITIALIZED,
      BOUND,
      LISTENING,
      CLOSED
    };

    SocketState m_sock_state;
    struct sockaddr_in m_address;
    int m_sock;
    int m_backlog;

    int m_kqueue;
    struct kevent m_event_subs;

    static const int EVENTS_MAX = 32;
    struct kevent m_event_list[EVENTS_MAX];

    static const int RECEIVE_MAX = 1024;
    char m_receive_buf[RECEIVE_MAX];

    int listen();
    int bind();
    int shutdown();
    int close();
  public:
    Transport(
        const char *addr = "0.0.0.0", 
        const int port = 8080);
    Transport(Transport &) = delete;
    Transport(Transport &&) = delete;
    ~Transport();

    int setup();

    void read_event();

    int on_read(struct kevent&);
    int on_eof(struct kevent&);

    int on_client_connect(struct kevent&);
    int on_client_disconnect(struct kevent&);
};

}

#endif // __TRANSPORT_H
