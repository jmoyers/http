clang++ -g -o build/tcp -std=c++11 -stdlib=libc++ -DDEBUG -Ivendor/sha1 tcp.cpp vendor/sha1/libsha.a && ./build/tcp
