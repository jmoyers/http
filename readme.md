* kqueue.c is a stupid small non-blocking http "server" to learn the api
* tcp.cpp is the same thing, poking around in c++
* parser.h is a toy http header parser
* going to be pushing on to websockets


networking api notes

```
server
  subscribe
    client connect
    client disconnect
    read
  transport (tcp) - support n. ideally tcp, udp with kqueue/epoll/completion 
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

```
