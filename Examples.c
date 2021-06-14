#include <stdio.h>

#include "Executor/Executor.h"

void test(TaskArgs _arguments) {
	bool cidc = ((bool*)(_arguments.cidc))[0];
	int i = ((int*)(_arguments.args))[0];
	int i100 = ((int*)(_arguments.args))[1];

	printf("test [%d:%d] helo.\n", i, i100);
}

void test2(TaskArgs _arguments) {
	int i = ((int*)(_arguments.args))[0];

	printf("test2 [%d] helo.\n", i);

}

void main() {
	const int poolSize = 10;
	const int nParallel = 3;


//*
	Executor* etor = NewExecutor(Process, poolSize, nParallel);

	int* args;
	if( (args = (int*)(malloc(sizeof(int) * 2))) == NULL ) {
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < poolSize + 3; i++) { // 3超過
	    args[0] = i;
		args[1] = i + 100;

		if ( etor->execute(etor, (Task){ test, (TaskArgs){ args } }) == -1 ) {
			printf("函数のExecutorへの登録に失敗\n");
		}
	}
	sleep(1);

	for (int i = 0; i < poolSize; i++) {
	    *args = i;

		if ( etor->execute(etor, (Task){ test2, (TaskArgs){ args } }) == -1 ) {
			printf("函数のExecutorへの登録に失敗\n");
		}
	}
	sleep(1);

	etor->stop(etor);
	etor->delete(etor);


/*/


	Executor* etor2 = NewExecutor(Thread, poolSize, nParallel);

	int* args2;
	if( (args2 = (int*)(malloc(sizeof(int)))) == NULL ) {
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < poolSize; i++) {
	    *args2 = i;

		if ( etor2->execute(etor2, (Task){ test2, (TaskArgs){ args2 } }) == -1 ) {
			printf("函数のExecutorへの登録に失敗\n");
		}
	}
	sleep(5);

	for (int i = 0; i < poolSize + 3; i++) { // 3超過
	    *args2 = i;

		if ( etor2->execute(etor2, (Task){ test2, (TaskArgs){ args2 } }) == -1 ) {
			printf("函数のExecutorへの登録に失敗\n");
		}
	}
	sleep(5);

	etor2->stop(etor2);
	etor2->delete(etor2);
//*/


}
