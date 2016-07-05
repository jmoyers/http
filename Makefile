CXXFLAGS = -std=c++11 -stdlib=libc++

DEBUG ?= 1

ifeq ($(DEBUG),1)
CXXFLAGS += -DLOG_LEVEL=DEBUG_LEVEL -O0 -g
else
CXXFLAGS += -Ofast
endif

SERVER_SRC = server.cpp
PARSER_SRC = parser.cpp
TRANSPORT_SRC = transport.cpp
SOCKET_SRC = socket.cpp
SERVER_RUN_SRC = main.cpp

PARSER_TESTS = tests/parser.cpp
SOCKET_TESTS = tests/socket.cpp
TESTS_INCLUDE = -Ivendor/bandit/ -I.

all: server parser main parser_tests

main: server parser
	$(CXX) -o build/server $(CXXFLAGS) \
		build/server.o build/parser.o $(SERVER_RUN_SRC)

server: parser
	$(CXX) -c -o build/server.o $(CXXFLAGS) $(SERVER_SRC)

parser:
	$(CXX) -c -o build/parser.o $(CXXFLAGS) $(PARSER_SRC)

transport: socket
	$(CXX) -c -o build/transport.o $(CXXFLAGS) $(TRANSPORT_SRC)

socket:
	$(CXX) -c -o build/socket.o $(CXXFLAGS) $(SOCKET_SRC)

socket_tests: socket
	$(CXX) -o build/tests/socket $(CXXFLAGS) $(TESTS_INCLUDE) $(SOCKET_TESTS) \
		build/socket.o
	build/tests/socket

parser_tests: parser
	$(CXX) -o build/tests/parser $(CXXFLAGS) \
		build/parser.o $(TESTS_INCLUDE) $(PARSER_TESTS)
	build/tests/parser
