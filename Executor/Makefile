AR = ar -qc

OBJS = Executor.o

../Executor.a: ${OBJS}
	${RM} $@
	${AR} $@ ${OBJS}

${OBJS}: Executor.h

clean:
	${RM} *.o
	${RM} ../Executor.a
