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
 * A connection is identified as a tuple of five values:
 * {<protocol>, <src addr>, <src port>, <dest addr>, <dest port>}
 *
 * Without SO_REUSEADDR, binding socketA to 0.0.0.0:21 and then binding 
 * socketB to 192.168.0.1:21 will fail (with error EADDRINUSE), since 
 * 0.0.0.0 means "any local IP address"
 */
Transport::Transport() :
  m_listen(Socket()),
  m_backlog(),
  m_kqueue(),
  m_event_subs(),
  m_event_list(),
  m_receive_buf()
{
}

int Transport::listen(const char *addr, int port)
{
  if (m_listen.configure(addr, port) < 0) return m_listen.err();
  if (m_listen.bind() < 0)                return m_listen.err();
  if (m_listen.listen() < 0)              return m_listen.err();

  if (m_listen.state() != Socket::State::LISTENING)
  {
    return m_listen.err(); 
  }

  m_kqueue = kqueue();

  EV_SET(&m_event_subs, m_listen.fd(), EVFILT_READ, EV_ADD, 0, 0, NULL);

  int err = kevent(m_kqueue, &m_event_subs, 1, NULL, 0, NULL);

  if (err < 0)
  {
    ERR("kqueue setup: %s", strerror(errno));
    ERR("  m_listen: %lu", m_listen.fd());
    ERR("  m_kqueue: %d", m_kqueue);
  }

  return err;
}

void Transport::pump()
{
  if (m_listen.state() != Socket::LISTENING)
  {
    ERR("must have a listening socket to read events, run setup");
    return;
  }

  int event_count = 0;
  int event_iter = 0;
  struct kevent event;

  event_count = kevent(m_kqueue, NULL, 0, m_event_list, EVENTS_MAX, NULL);

  if (event_count < 1)
  {
    ERR("kevent read: %s", strerror(errno));
    return;
  }

  for (event_iter = 0; event_iter < event_count; event_iter++)
  {
    event = m_event_list[event_iter];

    if (event.ident == m_listen.fd())
    {
      auto client = add_client(event);
      on_client_connect(client);
    }
    else
    {
      if (event.flags & EVFILT_READ) 
      {
        on_read(find_client(event));
      }
      if (event.flags & EV_EOF)
      {
        on_client_disconnect(find_client(event));
      }
    }
  }
}

Socket Transport::add_client(struct kevent &e)
{
  Socket client(e.ident);
  client.accept();

  m_clients[client.fd()] = client;

  return client;
}

Socket Transport::find_client(struct kevent &e)
{
  return m_clients[e.ident];
}

int Transport::on_client_connect(Socket client)
{
  client.accept();

  DEBUG("[0x%016" PRIXPTR "] client connect", (unsigned long) client.fd());

  if (client.err() < 0)
  {
    ERR("[0x%016" PRIXPTR "] client connect: %s", client.fd(), 
        strerror(errno));
  }

  // fill out of event subscription struct, so that we receive events for
  // the new client socket as well as our own
  EV_SET(&m_event_subs, client.fd(), EVFILT_READ, EV_ADD, 0, 0, NULL);
  
  // call kqueue with new event struct, registering out interests
  int err = kevent(m_kqueue, &m_event_subs, 1, NULL, 0, NULL);

  if (err < 0)
  {
    ERR("[0x%016" PRIXPTR  "] sub: %s", client.fd(), strerror(errno));
  }
  
  return err;
}

int Transport::on_client_disconnect(Socket client)
{
  DEBUG("[0x%016" PRIXPTR "] client disconnect", client.fd());

  // since we've been notified a client disconnected, unregister out interest
  EV_SET(&m_event_subs, client.fd(), EVFILT_READ, EV_DELETE, 0, 0, NULL);

  int err = kevent(m_kqueue, &m_event_subs, 1, NULL, 0, NULL);

  if (err < 0)
  {
    ERR("[0x%016" PRIXPTR "] kqueue unsub", client.fd());
  }

  // finally now that we don't receive events from kqueue, close the socket
  return client.close();
}

int Transport::on_read(Socket client)
{
  DEBUG("[1x%016" PRIXPTR "] client read", client.fd());

  int bytes = client.recv(m_receive_buf, RECEIVE_MAX);

  if (bytes <= 0)
  {
    ERR("[0x%016" PRIXPTR "] client receive: %s", client.fd(), 
        strerror(errno));
    return bytes;
  }

  DEBUG("received: %s", m_receive_buf);

  return bytes;
}

int Transport::close()
{
  m_listen.close();

  if (m_listen.err() < 0)
  {
    ERR("close: %s", strerror(errno));
  }

  return m_listen.err();
}

Transport::~Transport()
{
  if (m_listen.state() == Socket::LISTENING) close();
}

}
