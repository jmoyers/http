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

Server::Server(const char *addr, int port, int backlog) :
  m_address(),
  m_sock(),
  m_backlog(backlog),
  m_sock_reuse(1),
  m_kqueue(),
  m_event_subs(),
  m_event_list(),
  m_receive_buf(),
  m_sock_state(),
  m_clients()
{
  m_address.sin_family = AF_INET;
  m_address.sin_addr.s_addr = inet_addr(addr);
  m_address.sin_port = htons(port);

  m_sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

  if (m_sock < 0)
  {
    ERR("socket: %s", strerror(errno));
    return;
  }

  m_sock_state = INITIALIZED;
  m_sock_reuse = 1;

  setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &m_sock_reuse,
      sizeof(m_sock_reuse));

  fcntl(m_sock, F_SETFL, O_NONBLOCK);
}

int Server::bind()
{
  int err = ::bind(m_sock, (struct sockaddr *) &m_address, 
      sizeof(m_address));

  if (err < 0)
  {
    ERR("bind: %s", strerror(errno));
  }
  else
  {
    m_sock_state = BOUND;
  }

  return err;
}

int Server::listen()
{
  int err = ::listen(m_sock, m_backlog);

  if (err < 0)
  {
    ERR("listen: %s", strerror(errno));
  }
  else
  {
    m_sock_state = LISTENING;
  }

  return err;
}

int Server::setupRun()
{
  int err = 0;

  if (m_sock_state < BOUND)
  {
    if ((err = bind()) < 0) return err;
    if ((err = listen()) < 0) return err;
  }

  m_kqueue = kqueue();

  EV_SET(&m_event_subs, m_sock, EVFILT_READ, EV_ADD, 0, 0, NULL);

  err = kevent(m_kqueue, &m_event_subs, 1, NULL, 0, NULL);

  if (err < 0)
  {
    ERR("kqueue setup: %s", strerror(errno));
    ERR("  m_sock: %d", m_sock);
    ERR("  m_kqueue: %d", m_kqueue);
  }

  return err;
}

void Server::run()
{
  if (setupRun() < 0 && m_sock_state == LISTENING)
  {
    ERR("aborting run loop");
    return;
  }

  int event_count = 0;
  int event_iter = 0;
  struct kevent curr_event;

  for(;;)
  { 
    event_count = kevent(m_kqueue, NULL, 0, m_event_list, EVENTS_MAX, NULL);

    if (event_count < 1)
    {
      ERR("kevent read: %s", strerror(errno));
      return;
    }

    for (event_iter = 0; event_iter < event_count; event_iter++)
    {
      curr_event = m_event_list[event_iter];

      if (curr_event.ident == m_sock)
      {
        onClientConnect(curr_event);
      }
      else
      {
        if (curr_event.flags & EVFILT_READ) onRead(curr_event);
        if (curr_event.flags & EV_EOF)      onEOF(curr_event);
      }
    }
  }
}

int Server::onClientConnect(struct kevent& event)
{
  int client_sock = ::accept(event.ident, NULL, NULL);

  DEB("[0x%016" PRIXPTR "] client connect", (unsigned long) client_sock);

  if (client_sock < 0)
  {
    ERR("[0x%016" PRIXPTR "] client connect: %s", event.ident, 
        strerror(errno));
  }

  fcntl(client_sock, F_SETFL, O_NONBLOCK);

  EV_SET(&m_event_subs, client_sock, EVFILT_READ, EV_ADD, 0, 0, NULL);
  int err = kevent(m_kqueue, &m_event_subs, 1, NULL, 0, NULL);

  if (err < 0)
  {
    ERR("[0x%016" PRIXPTR  "] sub: %s", event.ident, strerror(errno));
  }
  
  return err;
}

int Server::onClientDisconnect(struct kevent& event)
{
  DEB("[0x%016" PRIXPTR "] client disconnect", event.ident);

  EV_SET(&m_event_subs, event.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);

  int err = kevent(m_kqueue, &m_event_subs, 1, NULL, 0, NULL);

  if (err < 0)
  {
    ERR("[0x%016" PRIXPTR "] kqueue unsub", event.ident);
  }

  return ::close(event.ident);
}

void Server::onRead(struct kevent& event)
{
  DEB("[0x%016" PRIXPTR "] client read", event.ident);

  const char *response = 
    "HTTP/1.1 200 OK\n"
    "Content-Type: text/html;\n\n"
    "Hello world!";

  int response_size = strlen(response) * sizeof(char);

  int bytes_read = recv(event.ident, m_receive_buf, 
      sizeof(m_receive_buf) - 1, 0);

  if (bytes_read <= 0)
  {
    ERR("[0x%016" PRIXPTR "] client receive: %s", event.ident, 
        strerror(errno));
    return;
  }

  m_receive_buf[bytes_read] = '\0';

  DEB("%s", m_receive_buf);

  Parser p(m_receive_buf, bytes_read);
  p.run();

  int bytes_sent = send(event.ident, response, response_size, 0);

  event.flags |= EV_EOF;
}

void Server::onEOF(struct kevent& event)
{
  DEB("[0x%016" PRIXPTR "] client eof", event.ident);

  onClientDisconnect(event);
}

int Server::close()
{
  int err = ::close(m_sock);

  if (err < 0)
  {
    ERR("close: %s", strerror(errno));
  }

  return err;
}

Server::~Server()
{
  if (m_sock_state == LISTENING) close();
}

int main() {
  (new Server())->run();
}
