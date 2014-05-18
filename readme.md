non-blocking hello world http server in c

- Purposese = personal education
- kqueue vs epoll vs poll vs select
- non-blocking vs blocking sockets
- ephemeral port exhaustion tests
- tested with ab -n 15k -c 130
- this c impl only 1.8~x faster than node!
- clang -O3, stdout removed
