#include <inttypes.h>
//#include "buffer.h"
#include <vector>

using Buffer = vector<uint8_t>;

class Client
{
  struct sockaddr m_addrress;
  int m_socket;

  Transport *m_transport;

  Buffer m_receive_buffer;

  Peer() : 
    m_address(),
    m_socket,
    m_transport,
    m_subscribers
  {}

  int send(Buffer &send_buffer, int size);
  int receive();
}
