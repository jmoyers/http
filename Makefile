SERVER_SRC=parser.cpp tcp.cpp main.cpp vendor/sha1/libsha.a
SERVER_INCLUDE=vendor/sha1

server:
	clang++ -g -o build/tcp -std=c++11 -stdlib=libc++ \
		-DLOG_LEVEL=DEBUG_LEVEL -i$(SERVER_INCLUDE) $(SERVER_SRC)
