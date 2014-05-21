#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <sys/event.h>
#include <inttypes.h>

int main(){
  struct sockaddr_in server_address;
  short server_port = 8080;
  char *server_ip = "10.1.10.11";
  int server_fd;

  if ((server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    fprintf(stderr, "Problem acquiring socket: %s\n", strerror(errno));
    exit(1);
  }

  int opt_reuse_addr = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt_reuse_addr, 
      sizeof(opt_reuse_addr));

  memset(&server_address, 0, sizeof(server_address));
  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = inet_addr(server_ip);
  server_address.sin_port = htons(server_port);

  if (bind(server_fd, (struct sockaddr *) &server_address, 
        sizeof(server_address)) < 0) {
    fprintf(stderr, "Problem binding to socket: %s\n", strerror(errno));
    exit(1);
  }

  if (listen(server_fd, 1000) < 0) {
    fprintf(stderr, "Problem setting fd to listen: %s\n", strerror(errno));
    close(server_fd);
    exit(1);
  }

  const int RECEIVE_SIZE = 1028;
  char receive_buf[RECEIVE_SIZE];
  int received_bytes = 1; 

  char *response = 
    "HTTP/1.1 200 OK\n"
    "Content-Type: text/html;\n\n"
    "Hello world!";

  int response_size = strlen(response) * sizeof(char);

  int kq = kqueue();
  struct kevent ev_set;
  struct kevent ev_list[32];
  int event_count, event_iter, client_fd;
  struct sockaddr_storage client_addr;
  socklen_t client_addr_len = sizeof(client_addr);

  EV_SET(&ev_set, server_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);

  if (kevent(kq, &ev_set, 1, NULL, 0, NULL) < 0) {
    fprintf(stderr, "Problem setting up read filter on kqueue: %s\n", 
        strerror(errno));
    close(server_fd);
    exit(1);
  }

  while (1) {
    if ((event_count = kevent(kq, NULL, 0, ev_list, 32, NULL)) < 1) {
      fprintf(stderr, "Problem asking for next kqueue event: %s\n", 
          strerror(errno));
      close(server_fd);
      exit(1);
    }

    printf("New events from the kernel. Count: %d\n", event_count);

    for (event_iter = 0; event_iter < event_count; event_iter++) {
      if (ev_list[event_iter].ident == server_fd) {
        printf("New connection, setting up accept\n");
        if ((client_fd = accept(ev_list[event_iter].ident, 
                (struct sockaddr *) &client_addr, &client_addr_len)) < 0) {
          fprintf(stderr, "Problem accepting new connection: %s\n", 
              strerror(errno));
        }

        setsockopt(client_fd, SOL_SOCKET, SO_REUSEADDR, &opt_reuse_addr, 
            sizeof(opt_reuse_addr));

        EV_SET(&ev_set, client_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);

        if (kevent(kq, &ev_set, 1, NULL, 0, NULL) < 0) {
          fprintf(stderr, "Problem adding kevent listener for client: %s\n",
              strerror(errno));
        }
      } else {
        if (ev_list[event_iter].flags & EVFILT_READ) {
          printf("Read event from client: 0x%016" PRIXPTR "\n", 
              ev_list[event_iter].ident);

          received_bytes = recv(ev_list[event_iter].ident, receive_buf, 
              sizeof(receive_buf), 0);

          if (received_bytes <= 0) {
            fprintf(stderr, "Error receiving bytes: %s\n", strerror(errno));
            close(ev_list[event_iter].ident);
            break;
          }
      
          printf("Read %d bytes from client: 0x%016" PRIXPTR "\n", 
              received_bytes, 
              ev_list[event_iter].ident);

          for (int i = 0; i < received_bytes; i++) {
            printf("%c", receive_buf[i]);
          }

          printf("\n");

          int bytes_sent = send(ev_list[event_iter].ident, 
              response, response_size, 0);

          printf("Sent %d/%d bytes to client: 0x%016" PRIXPTR "\n", bytes_sent, 
              response_size, ev_list[event_iter].ident);

          ev_list[event_iter].flags = ev_list[event_iter].flags ^ EV_EOF;
        } 
        
        if (ev_list[event_iter].flags & EV_EOF) {
          printf("EOF set for 0x%016" PRIXPTR "\n", ev_list[event_iter].ident);

          EV_SET(&ev_set, ev_list[event_iter].ident, EVFILT_READ, EV_DELETE, 
              0, 0, NULL);

          if (kevent(kq, &ev_set, 1, NULL, 0, NULL) < 0) {
            fprintf(stderr, "Problem removing kevent for client: %s\n", 
                strerror(errno));
            close(ev_list[event_iter].ident);
            exit(1);
          }

          close(ev_list[event_iter].ident);

          printf("Connection closed\n");
        }
      }
    }
  }

  return 0;
}
