CC     = gcc
CFLAGS = -std=gnu99 -ggdb3 -pthread -Wall

all: produtor-slock \
	consumidor-slock \
	produtor-sema \
	consumidor-sema

produtor-slock:src/produtor-slock.c
	@mkdir -p bin
	${CC} ${CFLAGS} -o ./bin/$@ $^

consumidor-slock:src/consumidor-slock.c
	@mkdir -p bin
	${CC} ${CFLAGS} -o ./bin/$@ $^

produtor-sema:src/produtor-sema.c
	@mkdir -p bin
	${CC} ${CFLAGS} -o ./bin/$@ $^

consumidor-sema:src/consumidor-sema.c
	@mkdir -p bin
	${CC} ${CFLAGS} -o ./bin/$@ $^

clean:
	rm -r bin
