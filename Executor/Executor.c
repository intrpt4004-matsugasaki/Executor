#include "Executor.h"

static bool _amIApprovedActor(Executor* self, const unsigned long id) {
	return self->_actorSignal == id;
}

static void _updateActorSignal(Executor* self) {
	if (self->_actorSignal >= self->_nParallel - 1)
			self->_actorSignal = 0;
	else
		self->_actorSignal++;
}

static void _generateActor(Executor* self, const unsigned long id) {
	self->_run = true;

	switch (self->_method) {
		case Process: {
			pid_t pid = fork();

			if (pid < 0) {
				fprintf(stderr, "[エラー]: プロセスの生成に失敗しました\n");
				exit(EXIT_FAILURE);
			} else if (pid > 0) {
				return;

			} else if (pid == 0) {
				while (self->_run) {
					/* 実処理 */
					// 未実行函数の取得&実行
					if (self->_amIApprovedActor(self, id)) {
						for (unsigned long i = 0; i < self->_poolSize; i++) {
							if (self->_pool[i].task.state == READY) {
								self->_pool[i].task.state = STARTED;
								self->_updateActorSignal(self);
								self->_pool[i].task.procedure(self->_pool[i].task.arguments);

								self->_pool[i].state = FREE;
							}
						}
					}
				}
				exit(EXIT_SUCCESS);
			}
		}

		case Thread: {
			/* ↓ 以下 SIGSEGV (Address boundary error) で動作しない 原因判らず ------------------------------------------------ */
			typedef struct {
				unsigned long id;
				Executor* self;
			} Act_d;

			void* act(void* _arguments) {
				Act_d* ad = (Act_d*)(_arguments);
				unsigned long id = ad->id;
				Executor* self = ad->self;

				while ( self->_run ) {
					/* 実処理 */
					// 未実行函数の取得&実行
					if (self->_amIApprovedActor(self, id)) {
						for (unsigned long i = 0; i < self->_poolSize; i++) {
							if (self->_pool[i].task.state == READY) {
								self->_pool[i].task.state = STARTED;
								self->_updateActorSignal(self);
								self->_pool[i].task.procedure(self->_pool[i].task.arguments);

								self->_pool[i].state = FREE;
							}
						}
					}
				}
			}

			self->_threads[id] = (pthread_t*)(malloc(sizeof(pthread_t)));

			Act_d ad;
			ad.id = id;
			ad.self = self;

			if ( pthread_create(self->_threads[id], NULL, act, &ad ) != 0 ) {
				fprintf(stderr, "[エラー]: スレッドの生成に失敗しました\n");
				exit(EXIT_FAILURE);
			}

			break;
			/* ↑ ここまで SIGSEGV -------------------------------------------------------------------------------------------- */
		}

		default:
			break;
	}
}

static int execute(Executor* self, Task task) {
	// 函数の登録
	for (unsigned long i = 0; i < self->_poolSize; i++) {
		if (self->_pool[i].state == FREE) {
			self->_pool[i].state = OCCUPIED;

			self->_pool[i].task.procedure = task.procedure;
			memcpy(self->_pool[i].task.arguments.args, task.arguments.args, sizeof(void*));
			self->_pool[i].task.state = READY;

			return 0;
		}
	}

	return -1;
}

// TODO: こちら側からの強制終了
static void stop(Executor* self) {
	if (!self->_run) {
		return;
	}
	self->_run = false;

	switch (self->_method) {
		case Process:
			for (unsigned long i = 0; i < self->_nParallel; i++)
				wait(NULL);

			break;

		case Thread:
			for (unsigned long id = 0; id < self->_nParallel; id++) {
				if (pthread_join(*(self->_threads[id]), NULL) != 0) {
					fprintf(stderr, "[エラー]: スレッドが止まりません\n");
					exit(EXIT_FAILURE);
				}
			}

			break;
	}

}

static void delete(Executor* self) {
	self->stop(self);

	if (self->_method == Thread) {
		for (unsigned long id = 0; id < self->_nParallel; id++)
			free(self->_threads[id]);
		free(self->_threads);
	}

	for (unsigned long i = 0; i < self->_poolSize; i++) {
		munmap(self->_pool[i].task.arguments.args, sizeof(void*));
	}
	munmap(self->_pool, sizeof(Field) * self->_poolSize);
	munmap(self, sizeof(Executor));
}

extern Executor* NewExecutor(const Method method, const unsigned long poolSize, const unsigned long nParallel) {
	Executor* executor = mmap(NULL, sizeof(Executor), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		executor->_amIApprovedActor = _amIApprovedActor;
		executor->_updateActorSignal = _updateActorSignal;
		executor->_generateActor = _generateActor;

		executor->execute = execute;
		executor->stop = stop;

		executor->delete = delete;

		executor->_method = method;

		executor->_poolSize = poolSize;

		executor->_pool = mmap(NULL, sizeof(Field) * executor->_poolSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
		for (unsigned long i = 0; i < executor->_poolSize; i++) {
			executor->_pool[i].task.arguments.args = mmap(NULL, sizeof(void*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
			executor->_pool[i].task.arguments.cidc = &executor->_run;

			executor->_pool[i].task.state = INITIALIZED;

			executor->_pool[i].state = FREE;
		}

		executor->_nParallel = nParallel;
		executor->_run = false;
		executor->_actorSignal = 0;

		switch (executor->_method) {
			case Thread:
				executor->_threads = (pthread_t**)(malloc(sizeof(pthread_t*) * executor->_nParallel));

				break;
		}

		for (unsigned long id = 0; id < executor->_nParallel; id++) {
			executor->_generateActor(executor, id);
		}

	return executor;
}
