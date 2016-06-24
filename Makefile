SERVER_SRC=parser.cpp server.cpp vendor/sha1/libsha.a
SERVER_INCLUDE=-Ivendor/sha1
SERVER_RUN_SRC=main.cpp

PARSER_TESTS=tests/parser.cpp
TESTS_INCLUDE=-Ivendor/bandit/ -I.

server:
	clang++ -g -o build/server -std=c++11 -stdlib=libc++ \
		-DLOG_LEVEL=DEBUG_LEVEL $(SERVER_INCLUDE) $(SERVER_SRC) $(SERVER_RUN_SRC)

parser_tests:
	clang++ -g -o build/tests/parser -std=c++11 -stdlib=libc++ \
		-DLOG_LEVEL=DEBUG_LEVEL $(SERVER_INCLUDE) $(TESTS_INCLUDE) \
		$(SERVER_SRC) $(PARSER_TESTS)
	build/tests/parser
