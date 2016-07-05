* kqueue.c is a stupid small non-blocking http "server" to learn the api
* tcp.cpp is the same thing, poking around in c++
* parser.h is a toy http header parser
* going to be pushing on to websockets


networking api notes
```
 Server transport state machine
 1. Acquire socket with socket(), set fd to nonblocking with fcntl()
 2. Bind socket to address/port with bind()
 3. Begin listening with listen()
 4. Subscribe to kqueue with kqueue(), EV_SET(), kevent()
 5. Loop for kqueue events with kevent()
  a. You can get an event with your socket set as ident, new client connected
  b. You can get read events which means you can read a client sent msg
  c. You can get an eof, which means the client has disconnected

 Client transport state machine
 1. Acquire socket with socket(), set fd to nonblocking with fcntl()
 2. Subscribe to kqueue for this socket with kqueue(), EV_SET(), kevent()
 3. Connect to server with socket, server addr with connect()
 4. Send a message to the server with write() and a buffer
 5. Loop for kqueue events with kevent()
  a. You can get read events for server returning traffic to you
  b. You can get an eof from the server, which means you close the socket

  In TCP, a socket represents a connections -- both send and receive.
  When you acquire a port to listen on, you get notified of new connections
  via kqueue with a new socket for you to accept, read from, and close as
  you will.

server
  purpose
    application layer
    listens on addr/ports
    track connected clients - RAII for sockets
    ? high level protocol events, e.g. get(path, req, res)
  subscribe - n listeners
    client connect
    client disconnect
    read
    eof

    ? http
      request
        client
        read
      response
        client
        send
        end
      verb(path, request, response)
  transport
    purpose
      abstract tcp/udp/etc, kqueue/epoll/completion
      acquire/dispose of sockets
      listen/send
      suitable for both a client or server

    socket (listening)
      socketstate
      descriptor

    bind 
    listen

    subscribe
      events - n listeners
        client connect
        client disconnect
        read

    clients
      address
      socket
        socketstate
        descriptor

    send - data, length, client
    close - client
    shutdown - client

    run
      kqueue loop
        client connect << notified a new client connecting
          call accept: get new client
            socket descriptor
            address
          notify: client connect - client
        read << notified of new data available on descriptor
          get socket from kqueue
          call recv
          event: read - client, data, length
        eof << notified of eof received on descriptor
          flags & EV_EOF
          currently we close()
          event: client disconnect - client


buffer
  purpose
    efficient
      hold buffer from network api with minimal copying
      reusable
      ? doesn't necessarily memset 0 (malloc not calloc)
    provide some bounds checking
    provide convenient interface for parser
    ? handles lists of buffers and indexes into them
  discuss
    std::string
      notes
        string vs wstring - http://goo.gl/hXOCVU
        ? string still not good at manipulating utf8?
        ? std::string can store unicode stuff, not manipulate (UTF8)
        ? std::wstring can manipulate (UCS-2)
      pros
        can handle multiple null terminators
        has bounds checking
        convenient for parser
        contiguous storage since C++0x (http://goo.gl/KJKQbB)
      cons
        .data() iand .c_str() are const
        consequently can't hand a buffer off to c api
        wide api surface for data that is not text
    std::vector<char|unsigned char|wchat_t|uint8_t>
      notes
        customize buffer class with template
          TCPTransport<std::vector<uint8_t>> - "concept" api
            .data() for pointer to 0
            subscript &data[0]
            size() for bounds checking
        ascii table - http://www.asciitable.com/
          0 to 127 non-extended
          128 - 255 extended
          ? non-extended fit in either char or unsigned char
        char
          char *test = "ol"
          c standard: char isn't guaranteed to be 8 bit
          c standard: char isn't guaranteed not to have padding bits
          in practice: signedness is always two's complement.
          in practice: an integer always uses all bits allocated.
          ? undefined behavior: if c is signed char[], c[0] = 0xF0
          ? due to signedness, not suitable for arbitrary bitwise operations
          unsigned vs signed as buffer - http://goo.gl/XTc00s
        unsigned char
          http://goo.gl/1Zwr7M
          no padding bits
          bitwise produce no undefined behavior, trap representations, overflow
          trap representation - http://goo.gl/eQpIBi
            bit value stored in var of type x where if used as value cause UB
          can alias any data type
        uint8_t
          guaranteed to be 8 bit
          works with shifts, unary, etc
          ? not guaranteed to exist on some embedded systems
          ? makes parsing weird, can't do buf[index] == 'a'
        wchar_t
          good for holding unicode
          wchar_t = 2 bytes on windows, 4 byte on linux
          wchar_t is not tied to unicode, but guarantees storage
        bool
          has a specialization for extremely compact storage
          http://www.cplusplus.com/reference/vector/vector-bool/
          not good for processing
          variable size bitset
      pros
        .reserve() and .resize() make it resizable
        bounds checking
        .data() provides a nice underlying block of contiguous memory
        can hand off &vec[0] or .data() to c functions like recv
      cons
        slightly less convenient for parser
        std::string(v.data(), v.size()) is a copy, no safe way without a copy
    custom buffer
      needs a pointer, a size, and an offset
      very simple
      could use realloc, memcpy directly
      could implement << and >> and other stream interfaces
      could implement .begin() and .end() for range for
      iterators?

client
  purpose
    be able to store list of connected clients
    holds address, socket, socketstate, which transport
    part of allowing server to be io api agnostic (kqueue, epoll, completion)

  socket
    socketstate
    desriptor
  address

  Does send/recv/transport even belong here?
  ? bound_tranport
  ? stream_in
  ? stream_out
  ? send
  ? recv

event - abstract kqueue events, api compatible with epoll
  subscriber
    std::function(client, flags)

performance
  tools
    gperftools (google) - http://goo.gl/GKT8D7
  slab allocators
    why?
      still reading
    "Memory Allocation for Long-Running..." - http://goo.gl/IRME5u
    jemalloc (canonware) - http://goo.gl/4wYkgS
    jemalloc (facebook) - http://goo.gl/DX79Yk
    jemalloc tech talk - http://goo.gl/2YLWTx, alt: http://goo.gl/fsdZ3n
    tcmalloc (google) - http://goo.gl/GKT8D7
    data locality - http://goo.gl/y6SsFi
    locality of reference - http://goo.gl/tyk6uV
    "Memory allocator microbenchmark results are notoriously difficult to extrapolate to real-world applications"
```
