#include "types.h"
#include "user.h"
#include "x86.h"

#define PAGESIZE      4096
#define NULL ((void *)0)


struct mymcsnode {
	struct mymcsnode *next;
	uint locked;
	uint isnull;
};

struct mymcslock {
	// struct mymcsnode front;
	struct mymcsnode *tail;
};

struct frisbee_mcs{
	uint value;
	uint count;
	struct mymcxlock mcslock;
};

uint threads;
uint passes;
struct frisbee_mcs token;
// struct mymcsnode nullnode;

void init_mymcslock(struct mymcslock *lk){
	// node->isnull = 1;
	lk->tail = NULL;
}

void acquire_mcslock(struct mymcslock *lk, struct mymcsnode *node){
	struct mymcsnode *prenode;
	
	node->next = NULL;

	prenode = xchg(&lk->tail, node);
	__sync_synchronize();

	if(prenode != NULL){
		node->locked = 1;
		prenode.next = node;
		while(node->locked)
			;
	}

}

void release_mcslock(struct mymcslock *lk, struct mymcsnode *node){
	// __sync_synchronize();

	// struct mymcsnode *tmp = node;
	// if(node->next == NULL){
	// 	if(cmpxchg(lk->tail, tmp, NULL) == node){
	// 		return;
	// 	}
	// 	else{
	// 		while (node->next == NULL)
	// 			;
	// 	}
	// }
	// node->locked = 0;
}

void start_routine(void *arg){
	struct mymcsnode mynode;
	// mynode.locked = 0;
	mynode.next = NULL;

	int* threadid = (int *)arg;
	int next;
	if(*threadid < threads-1)
		next = *threadid + 1;
	else
		next = 0;

	while(token.count < passes){
		if(token.count > passes){
			break;
		}
		acquire_mcslock(&token.mcslock, &mynode);

		if(token.value == *threadid && token.count <= passes){
			token.value = next;
			printf(1, "Pass number no: %d, Thread %d is passing the to thread %d\n",token.count, *threadid, next);	
			token.count++;
		}

		release_mcslock(&token.mcslock, &mynode);
		
	}

	exit();
}


int main(int argc, char *argv[]){

	int start, finish;
	start = uptime();

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
	token.value = 0;
	init_mymcslock(&token.mcslock);



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

	finish = uptime();
	printf(1, "running time : %d\n", (finish-start));
	exit();

	return 0;
}


