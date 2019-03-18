#include "types.h"
#include "user.h"

#define PAGESIZE      4096

void start_routine(void *arg){
	// int thisid = getpid();
	int* gett = (int *)arg;

	sleep(10);
	printf(1, "sr: arg is %d, pid is %d\n", *gett, getpid());
	exit();
}

// routine to create thread with function for thread to execute
int thread_create(void (*funcp)(void*), void *arg, int threads, int passes){

	int *stack_pointer;
	int pid;
	int i;

	void *stacks[threads];
	int *args[threads];

// prepare the stack
	for (i = 0; i < threads; i++){
		stacks[i] = (void*) malloc(PAGESIZE);
		if (!stacks[i]) {
			printf(2, "failed to allocate stack for thread");
		}

		args[i] = (int*) malloc(4);
		*args[i] = i;

		stack_pointer = stacks[i] + PAGESIZE - sizeof(int *);
		*stack_pointer = (int)args[i];

	}
// create threads
	for (i = 0; i < threads; i++){
		pid = clone(funcp, stacks[i], PAGESIZE);
		printf(1, "thread_create: pid is %d\n", pid);
	}

// collect threads
	for (i = 0; i < threads; i++){
		pid = wait();
		printf(1, "thread collect: %d\n", pid);
	}

	exit();

	return 0;
}

int main(int argc, char *argv[]){

	if(argc <= 2){
		printf(2, "try again please\n");
		exit();
	}

	int threads;
	int passes;

	threads = atoi(argv[1]);
	passes = atoi(argv[2]);

	printf(1, "threads: %d, passes: %d\n", threads, passes);

	void start_routine(void *);
	void *stacks[threads];
	int *args[threads];
	int i;

	// prepare the stack
	for (i = 0; i < threads; i++){
		stacks[i] = (void*)malloc(PAGESIZE);
		args[i] = (int*) malloc(4);
		*args[i] = i;

		int *stack_pointer;
		stack_pointer = stacks[i] + PAGESIZE - sizeof(int *);
		*stack_pointer = (int)args[i];
	}

	for (i = 0; i < threads; i++){
		int pid = clone(start_routine, stacks[i], PAGESIZE);
		printf(1, "create: pid is %d\n", pid);
	}

	sleep(100);
	for (i = 0; i < threads; i++){
		wait();
	}

	exit();

	// thread_create(start_routine, testarg, threads, passes);
	// start_routine(testarg);

	return 0;
}



