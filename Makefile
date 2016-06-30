SERVER_SRC=server.cpp
PARSER_SRC=parser.cpp
SERVER_RUN_SRC=main.cpp

PARSER_TESTS=tests/parser.cpp
TESTS_INCLUDE=-Ivendor/bandit/ -I.

main: server parser
	clang++ -g -o build/server -std=c++11 -stdlib=libc++ \
		-DLOG_LEVEL=DEBUG_LEVEL build/server.o build/parser.o $(SERVER_RUN_SRC)

server: parser
	clang++ -g -c -o build/server.o -std=c++11 -stdlib=libc++ \
		-DLOG_LEVEL=DEBUG_LEVEL $(SERVER_SRC)

parser:
	clang++ -g -c -o build/parser.o -std=c++11 -stdlib=libc++ \
		-DLOG_LEVEL=DEBUG_LEVEL $(PARSER_SRC)

parser_tests: parser
	clang++ -g -o build/tests/parser -std=c++11 -stdlib=libc++ \
		-DLOG_LEVEL=DEBUG_LEVEL build/parser.o $(TESTS_INCLUDE) $(PARSER_TESTS)
	build/tests/parser
