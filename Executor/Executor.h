#ifndef Executor_h
#define Executor_h

#include <sys/mman.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>


typedef enum {
	Process,
	Thread
} Method;

typedef enum {
	INITIALIZED,
	READY,
	STARTED,
} TaskState;

typedef struct {
	void* args;
	bool* cidc;
} TaskArgs;

typedef struct {
	void (* procedure)(TaskArgs);
	TaskArgs arguments;
	TaskState state;
} Task;

typedef enum {
	FREE,
	OCCUPIED
} FieldState;

typedef struct {
	Task task;
	FieldState state;
} Field;

typedef struct Executor {
	Method _method;

	unsigned long _poolSize;
	Field* _pool;

	unsigned long _nParallel;
	bool _run;
	unsigned long _actorSignal;

	pthread_t** _threads;

	bool (* _amIApprovedActor)(struct Executor* self, const unsigned long id);
	void (* _updateActorSignal)(struct Executor* self);
	void (* _generateActor)(struct Executor* self, unsigned long id);

	int (* execute)(struct Executor* self, Task task);
	void (* stop)(struct Executor* self);

	void (* delete)(struct Executor* self);
} Executor;

extern Executor* NewExecutor(const Method method, const unsigned long nParallel, const unsigned long poolSize);

#endif
