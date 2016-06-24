#ifndef __SERVER_H
#define __SERVER_H

#include <string>
#include <unordered_map>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/event.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <fcntl.h>
#include "log.h"
#include "parser.h"

class Server
{
  public:
    Server(
        const char *addr = "0.0.0.0", 
        const int port = 8080, 
        const int backlog = 1000);
    Server(Server &s) = delete;
    Server(Server &&s) = delete;
    ~Server();

    void onRead(struct kevent& event);
    void onEOF(struct kevent& event);

    int onClientConnect(struct kevent& event);
    int onClientDisconnect(struct kevent& event);

    void run();
  private:
    int bind();
    int listen();
    int shutdown();
    int close();

    int setupRun();

    struct sockaddr_in m_address;
    int m_sock_reuse;
    int m_sock;
    int m_backlog;

    int m_kqueue;
    struct kevent m_event_subs;
    static const int EVENTS_MAX = 32;
    struct kevent m_event_list[EVENTS_MAX];

    static const int RECEIVE_MAX = 1024;
    char m_receive_buf[RECEIVE_MAX];

    enum SocketState {
      INITIALIZED,
      BOUND,
      LISTENING,
      CLOSED
    };

    SocketState m_sock_state;

    std::unordered_map<std::string, std::string> m_clients;
};

#endif /** __SERVER_H **/
