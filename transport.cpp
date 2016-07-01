#include "transport.h"

namespace Http
{

/**
 * On construct, we take the address and port for listening and acquire
 * a tcp socket using the berkely socket api.
 *
 * If this socket aqusition, we move the socket state machine forward
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


}
