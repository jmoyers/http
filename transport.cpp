#include "transport.h"

namespace Net
{

/**
 * On construct, we take the address and port for listening and acquire
 * a tcp socket using the berkely socket api.
 *
 * If the socket is acquired, we move the socket state machine forward
 * to initialized.
 *
 * A socket is identified as a tuple of five values:
 * {<protocol>, <src addr>, <src port>, <dest addr>, <dest port>}
 *
 * Without SO_REUSEADDR, binding socketA to 0.0.0.0:21 and then binding 
 * socketB to 192.168.0.1:21 will fail (with error EADDRINUSE), since 
 * 0.0.0.0 means "any local IP address"
 */
Transport::Transport(const char *addr, int port) :
  m_address(),
  m_sock(),
  m_backlog(1000),
  m_kqueue(),
  m_event_subs(),
  m_event_list(),
  m_receive_buf(),
  m_sock_state()
{
  // For the curious mind
  // http://stackoverflow.com/q/6729366/
  m_address.sin_family = AF_INET;

  // Converts a string containing an ipv4 dotted-decimal address into
  // what this struct and the api is expecting
  m_address.sin_addr.s_addr = inet_addr(addr);

  // Converts the unsigned integer port from host byte order to 
  // network byte order, little endian to big endian
  m_address.sin_port = htons(port);

  //  AF_INET - Address family
  //  SOCK_STREAM - Stream based protocl (as opposed to datagram UDP)
  //  IPPROTO_TCP - Explicitly specifies TCP protocol, though it sounds
  //    as though we could just specify 0 here and SOCK_STREAM would force
  m_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

  // Berkeley socket API returns integers and less than 0 is an error
  if (m_sock < 0)
  {
    ERR("socket: %s", strerror(errno));
    return;
  }

  m_sock_state = INITIALIZED;

  int sock_reuse = 1;
  setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &sock_reuse,
      sizeof(sock_reuse));

  // fcntl = change file descriptor. a socket is a file descriptor.
  // without setting this to non-block, if you use a function that
  // receives or sends data, it will wait until it gets something.
  //
  // if we set this, one technique, which we will use, it to use
  // kqueue to let the kernel notify us when a new event comes through
  // for the socket.
  fcntl(m_sock, F_SETFL, O_NONBLOCK);
}

int Transport::bind()
{
  // Once socket is initialized with protocol type, and nonblocking fd options
  // from the constructor, we can bind the address
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

int Transport::listen()
{
  // Once the socket is bound, we can then listen on the socket. The kernel
  // also wants a backlog, which is the max pending connections for the socket
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

int Transport::setup()
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

void Transport::read_event()
{
  if (m_sock_state != LISTENING)
  {
    ERR("must have a listening socket to read events, run setup");
    return;
  }

  int event_count = 0;
  int event_iter = 0;
  struct kevent curr_event;

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
      on_client_connect(curr_event);
    }
    else
    {
      if (curr_event.flags & EVFILT_READ) on_read(curr_event);
      if (curr_event.flags & EV_EOF)      on_client_disconnect(curr_event);
    }
  }
}

int Transport::on_client_connect(struct kevent& event)
{
  // we were notified of client connection by kqueue, so accept the connection
  int client_sock = ::accept(event.ident, NULL, NULL);

  DEBUG("[0x%016" PRIXPTR "] client connect", (unsigned long) client_sock);

  if (client_sock < 0)
  {
    ERR("[0x%016" PRIXPTR "] client connect: %s", event.ident, 
        strerror(errno));
  }

  // set the new client connection to non-blocking
  fcntl(client_sock, F_SETFL, O_NONBLOCK);

  // fill out of event subscription struct, so that we receive events for
  // the new client socket as well as our own
  EV_SET(&m_event_subs, client_sock, EVFILT_READ, EV_ADD, 0, 0, NULL);
  
  // call kqueue with new event struct, registering out interests
  int err = kevent(m_kqueue, &m_event_subs, 1, NULL, 0, NULL);

  if (err < 0)
  {
    ERR("[0x%016" PRIXPTR  "] sub: %s", event.ident, strerror(errno));
  }
  
  return err;
}

int Transport::on_client_disconnect(struct kevent& event)
{
  DEBUG("[0x%016" PRIXPTR "] client disconnect", event.ident);

  // since we've been notified a client disconnected, unregister out interest
  EV_SET(&m_event_subs, event.ident, EVFILT_READ, EV_DELETE, 0, 0, NULL);

  int err = kevent(m_kqueue, &m_event_subs, 1, NULL, 0, NULL);

  if (err < 0)
  {
    ERR("[0x%016" PRIXPTR "] kqueue unsub", event.ident);
  }

  // finally now that we don't receive events from kqueue, close the socket
  return ::close(event.ident);
}

int Transport::on_read(struct kevent& event)
{
  DEBUG("[0x%016" PRIXPTR "] client read", event.ident);

  int bytes_read = recv(event.ident, m_receive_buf, 
      sizeof(m_receive_buf) - 1, 0);

  if (bytes_read <= 0)
  {
    ERR("[0x%016" PRIXPTR "] client receive: %s", event.ident, 
        strerror(errno));
    return bytes_read;
  }

  m_receive_buf[bytes_read] = '\0';

  DEBUG("received: %s", m_receive_buf);

  // lets just be an echo server for right now
  std::string response(m_receive_buf, strlen(m_receive_buf));
  int bytes_sent = send(event.ident, response.c_str(), response.size(), 0);

  event.flags |= EV_EOF;
  return bytes_read;
}

int Transport::close()
{
  int err = ::close(m_sock);

  if (err < 0)
  {
    ERR("close: %s", strerror(errno));
  }

  return err;
}

Transport::~Transport()
{
  if (m_sock_state == LISTENING) close();
}

}
