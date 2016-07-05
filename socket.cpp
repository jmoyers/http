#include "socket.h"

Socket::Socket(FD fd, State state = INVALID)
  : m_fd(fd),
    m_state(state),
    ipv4(),
    m_buf()
{
}

Socket::Socket()
  : m_listen_addr(),
    m_send_addr(),
    m_buf()
}

static void Socket::ipv4(IPV4 &ipv4, const char *addr, int &port)
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
  Socket::ipv4(&m_listen_addr, addr, port);
  return configure();
}

int Socket::configure()
{
  //  AF_INET - Address family
  //  SOCK_STREAM - Stream based protocl (as opposed to datagram UDP)
  //  IPPROTO_TCP - Explicitly specifies TCP protocol
  m_fd = socket(AF_INET, SOCK_STREAM, IPROTO_TCP); 

  if (m_fd < 0)
  {
    ERR("socket: %s", strerror(errno));
    return m_fd;
  }


  // allow binding to wildcard addresses, while also using specific addresses
  // so 0.0.0.0:80 and 192.168.0.1:80 are two different addresses
  setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &m_reuse,
      sizeof(int));

  // fcntl = change file descriptor. a socket is a file descriptor.
  // without setting this to non-block, if you use a function that
  // receives or sends data, it will wait until it gets something.
  //
  // if we set this, one technique, which we will use, it to use
  // kqueue to let the kernel notify us when a new event comes through
  // for the socket.
  fcntl(m_sock, F_SETFL, O_NONBLOCK);

  m_state = INITIALIZED;

  return m_fd;
}

int Socket::bind()
{
  m_err = ::bind(m_fd, (struct sockaddr *) &m_listen_addr,
      sizeof(ipv4_addr));

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
  ::accept(m_fd, NULL, NULL);
  
  // set the new client connection to non-blocking
  fcntl(m_fd, F_SETFL, O_NONBLOCK);
}

int Socket::close()
{
  ::close(m_fd);
}

int Socket::send(const char *buf, size_t length)
{
  return ::send(m_fd, buf, length, 0);
}

int Socket::recv(char *buf, size_t length)
{
  return ::recv(m_fd, buf, length, 0);
}
