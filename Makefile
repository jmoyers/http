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
SERVER_RUN_SRC = main.cpp

PARSER_TESTS = tests/parser.cpp
TESTS_INCLUDE = -Ivendor/bandit/ -I.

all: server parser main parser_tests

main: server parser
	$(CXX) -o build/server $(CXXFLAGS) \
		build/server.o build/parser.o $(SERVER_RUN_SRC)

server: parser
	$(CXX) -c -o build/server.o $(CXXFLAGS) $(SERVER_SRC)

parser:
	$(CXX) -c -o build/parser.o $(CXXFLAGS) $(PARSER_SRC)

transport:
	$(CXX) -c -o build/transport.o $(CXXFLAGS) $(TRANSPORT_SRC)

parser_tests: parser
	$(CXX) -o build/tests/parser $(CXXFLAGS) \
		build/parser.o $(TESTS_INCLUDE) $(PARSER_TESTS)
	build/tests/parser
