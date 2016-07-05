#ifndef __TRANSPORT_H
#define __TRANSPORT_H

#include <sys/event.h>  // kevent
#include <string>       // std::string
#include <inttypes.h>   // PRIXPTR (printing pointers)
#include <unordered_map>

#include "socket.h"
#include "log.h"

namespace Net
{

class Transport {
  protected:
    std::unordered_map<Socket::FD, Socket> m_clients;
    Socket m_listen;
    int m_backlog;

    int m_kqueue;
    struct kevent m_event_subs;

    static const int EVENTS_MAX = 32;
    struct kevent m_event_list[EVENTS_MAX];

    int bind();
    int shutdown();
    int close();
  public:
    Transport();
    Transport(Transport &) = delete;
    Transport(Transport &&) = delete;
    ~Transport();

    int send(Socket, const char *, size_t);

    Socket find_client(struct kevent&);
    Socket add_client(struct kevent&);

    int on_read(struct kevent&);
    int on_eof(struct kevent&);

    // listen - "server" specific
    Socket listen(const char*, int);  
    Socket on_client_connect(struct kevent&);
    Socket on_client_disconnect(struct kevent&);

    void pump();
};

}

#endif // __TRANSPORT_H
