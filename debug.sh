clang++ -g -o build/tcp -std=c++11 -stdlib=libc++ -DLOG_LEVEL=ERR_LEVEL -Ivendor/sha1 parser.cpp tcp.cpp main.cpp vendor/sha1/libsha.a && ./build/tcp
