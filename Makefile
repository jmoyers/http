debug: CFLAGS = -DDEBUG -g
debug: pc_server

release: CFLAGS += -O3
release: pc_server

pc_server: pc_server.c
	clang $(CFLAGS) -o pc_server pc_server.c

clean: 
	rm -f pc_server
	rm -rf pc_server.dSYM
