SHELL   = /bin/bash
CC      = gcc
CFLAGS  = -std=gnu99 -ggdb3 -lpthread -Wall

OUTDIR  = ./bin
SRCDIR  = ./src
TARGETS = $(addprefix ${OUTDIR}/,prod-lck cons-lck prod-sem cons-sem)

.PHONY: all
all: ${TARGETS}

${OUTDIR}/%: ${SRCDIR}/%.c
	${CC} ${CFLAGS} -o $@ $<

${TARGETS}: | ${OUTDIR}

${OUTDIR}:; $([ -d $@ ] || mkdir $@)

.PHONY: clean
clean:
	rm -r ${OUTDIR}
