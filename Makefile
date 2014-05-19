debug: CFLAGS = -DDEBUG -g
debug: kqueue

release: CFLAGS += -O3
release: kqueue

kqueue: kqueue.c
	clang $(CFLAGS) -o kqueue kqueue.c

clean: 
	rm -f kqueue
	rm -rf kqueue.dSYM
