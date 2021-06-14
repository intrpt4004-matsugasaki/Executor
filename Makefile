CC = gcc

P = Examples
default: ${P}

OBJS = Examples.c
${P}: Executor.a
	${CC} ${OBJS} -lpthread Executor.a -L. -o $@

Executor.a:
	make -C Executor

clean:
	${RM} ${P}
	make clean -C Executor

