#include "types.h"
#include "user.h"
#include "x86.h"
// #include "myspinlock.h"

#define PAGESIZE      4096

struct myspinlock {
  uint locked;       // Is the lock held?
};

struct frisbee {
	uint value;
	struct myspinlock lock;
	uint count;
};

uint threads;
uint passes;
struct frisbee token;


void init_myspinlock(struct myspinlock *lk){
	lk->locked = 0;
}

void acquire_spinlock(struct myspinlock *lk){
	while(xchg(&lk->locked, 1) != 0)
		;
	__sync_synchronize();
}

void release_spinlock(struct myspinlock *lk){
	__sync_synchronize();
	asm volatile("movl $0, %0" : "+m" (lk->locked) : );
}

void start_routine(void *arg){
	int* threadid = (int *)arg;
	int next;
	sleep(10);
	if(*threadid < threads-1)
		next = *threadid + 1;
	else
		next = 0;

	while(token.count < passes){
		if(token.count > passes){
			break;
		}
		acquire_spinlock(&token.lock);

		if(token.value == *threadid && token.count <= passes){
			token.value = next;
			printf(1, "Pass number no: %d, Thread %d is passing the token to thread %d\n",token.count, *threadid, next);	
			token.count++;
		}
		release_spinlock(&token.lock);
		
	}

	exit();
}


int main(int argc, char *argv[]){

	if(argc <= 2){
		printf(2, "try again please\n");
		exit();
	}

	threads = atoi(argv[1]);
	passes = atoi(argv[2]);

	printf(1, "threads: %d, passes: %d\n", threads, passes);

	void start_routine(void *);
	void *stacks[threads];
	int *args[threads];
	int i;

	token.count = 1;
	init_myspinlock(&token.lock);
	token.value = 0;

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

	sleep(20);
	for (i = 0; i < threads; i++){
		wait();
		// printf(1, "token.count is %d\n", token.count);
	}

	exit();

	

	return 0;
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
