#include "types.h"
#include "user.h"

#define PAGESIZE      4096


int main(int argc, char *argv[]){
	int *testarg;
	int pid;

	*testarg = 233;

	void (*fp) (void *arg);
	fp = start_routine;
	pid = thread_create(fp, testarg);

	printf("main: pid is %d\n", pid);

	wait();
	
	exit();
}

// routine to create thread with function for thread to execute
int thread_create(void *(*start_routine)(void*), void *arg){

	int *stack_pointer;
	int pid;

	// store the arguments in the stack
	void *stack = (void*) malloc(PAGESIZE);
	stack_pointer = stack + PAGESIZE - sizeof(int *);
	*stack_pointer = start_routine;
	
	stack_pointer -= sizeof(int *);
	*stack_pointer = (int)arg;

	pid = clone(stack, PAGESIZE);

	return pid;
}

void start_routine(void *arg){
	printf("sr: arg is %d, pid is %d\n", (int)arg, getpid());
	exit();
}