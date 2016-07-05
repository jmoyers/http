#include "socket.h"

namespace Net
{

Socket::Socket(FD fd, State state, Type type)
  : m_fd(fd),
    m_listen_addr(),
    m_send_addr(),
    m_type(type),
    m_state(state),
    m_buf()
{
  DEBUG("construct: %zu, %d, %d", m_fd, m_state, m_type);
}

Socket::Socket(Type type)
  : m_listen_addr(),
    m_type(type),
    m_send_addr(),
    m_buf()
{
  DEBUG("construct: %d", m_type);
}

void Socket::ipv4(IPV4 &ipv4, const char *addr, int &port)
{
  // For the curious mind
  // http://stackoverflow.com/q/6729366/
  ipv4.sin_family = AF_INET;

  // Converts a string containing an ipv4 dotted-decimal address into
  // what this struct and the api is expecting
  ipv4.sin_addr.s_addr = inet_addr(addr);

  // Converts the unsigned integer port from host byte order to 
  // network byte order, little endian to big endian
  ipv4.sin_port = htons(port);
}

int Socket::configure(const char *addr, int port)
{
  Socket::ipv4(m_listen_addr, addr, port);
  return configure();
}

int Socket::configure()
{
  //  AF_INET - Address family
  //  SOCK_STREAM - Stream based protocl (as opposed to datagram UDP)
  //  IPPROTO_TCP - Explicitly specifies TCP protocol
  m_err = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 

  if (m_err < 0)
  {
    ERR("socket: %s", strerror(errno));
    return m_err;
  }

  m_fd = m_err;

  // allow binding to wildcard addresses, while also using specific addresses
  // so 0.0.0.0:80 and 192.168.0.1:80 are two different addresses
  m_err = setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &m_reuse,
      sizeof(int));

  if (m_err < 0) 
  {
    ERR("setsockopt: %s", strerror(errno));
    return m_err;
  }

  // fcntl = change file descriptor. a socket is a file descriptor.
  // without setting this to non-block, if you use a function that
  // receives, sends, or connects it will wait until it potentially finishes.
  // I have read that it might only block until 1 byte is sent/received.
  //
  // if we set this, one technique, which we will use, it to use
  // kqueue to let the kernel notify us when a new event comes through
  // for the socket.
  if (m_type == NONBLOCKING)
  {
    m_err = fcntl(m_fd, F_SETFL, O_NONBLOCK);

    if (m_err < 0) 
    {
      ERR("fcntl: %s", strerror(errno));
      return m_err;
    }
  }

  m_state = INITIALIZED;

  return m_fd;
}

int Socket::bind()
{
  m_err = ::bind(m_fd, (struct sockaddr *) &m_listen_addr,
      sizeof(IPV4));

  if (m_err < 0)
  {
    m_state = INVALID;
    ERR("bind: %s", strerror(errno));
  }
  else
  {
    m_state = BOUND;
  }

  return m_err;
}


int Socket::listen()
{
  // Once the socket is bound, we can then listen on the socket. The kernel
  // also wants a backlog, which is the max pending connections for the socket
  m_err = ::listen(m_fd, m_backlog);

  if (m_err < 0)
  {
    m_state = INVALID;
    ERR("listen: %s", strerror(errno));
  }
  else
  {
    m_state = LISTENING;
  }

  return m_err;
}

int Socket::accept()
{
  // last two params are structs we need to use to get the client address
  // struct addr, and len
  m_err = ::accept(m_fd, NULL, NULL);

  if (m_err < 0) 
  {
    ERR("accept: %s", strerror(errno));
    return m_err;
  }

  m_accepted = m_err;
  
  // set the new client connection to non-blocking
  if (m_type == NONBLOCKING)
  {
    m_err = fcntl(m_accepted, F_SETFL, O_NONBLOCK);
    
    if (m_err < 0)
    {
      ERR("fcntl: %s", strerror(errno));
      m_state = INVALID;
      return m_err;
    } 
  }

  return m_accepted;
}

int Socket::close()
{
  DEBUG("close: %lu", fd());
  
  if (m_state != INVALID) 
  {
    m_err = ::close(fd());

    if (m_err < 0) 
    {
      ERR("close: %s", strerror(errno));
      m_state = INVALID;
    } 
    else
    {
      m_state = CLOSED;
    }
  }

  return m_err;
}

int Socket::connect(const char *addr, int port)
{
  Socket::ipv4(m_send_addr, addr, port);

  m_err = ::connect(m_fd, (struct sockaddr *) &m_send_addr, sizeof(IPV4));

  // EINPROGRESS returned when socket is nonblocking
  if (m_err < 0 && errno != EINPROGRESS)
  {
    ERR("connect: %s", strerror(errno));
    m_state = INVALID;
    return m_err;
  }

  if (m_err < 0 && errno == EINPROGRESS)
  {
    m_state = CONNECTING;
  }
  else
  {
    m_state = CONNECTED;
  }

  return m_err;
}

int Socket::send(const char *buf, size_t length)
{
  m_err = ::send(m_fd, buf, length, 0);

  if (m_err < 0)
  {
    ERR("send: %s", strerror(errno));
  }

  // bytes sent
  return m_err;
}

int Socket::recv(char *buf, size_t length)
{
  m_err = ::recv(m_fd, buf, length, 0);

  if (m_err < 0)
  {
    ERR("recv: %s", strerror(errno));
  }

  // bytes received
  return m_err;
}

}
