#include "types.h"
#include "user.h"
#include "x86.h"

#define PAGESIZE      4096
#define NULL ((void *)0)


struct mymcsnode {
	struct mymcsnode *next;
	volatile uint locked;
	uint id;
};

struct mymcslock {
	struct mymcsnode *tail;
};

struct frisbee_mcs{
	uint value;
	uint count;
	struct mymcslock mcslock;
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
	int * nullnode;

	// printf(1, "thread %d start acquire\n", node->id);
	
	node->next = NULL;

	int tmp = xchg((uint *)&lk->tail, (int)node);
	nullnode = (int *)tmp;
	prenode = (struct mymcsnode *)tmp;

	// if(){
	// 	printf(1, "zero-prenode id is %d\nis zero, now thread is %d\n", prenode->id, node->id);
	// }

	if(*nullnode == 0){
		node->locked = 1;
		prenode->next = node;
		// printf(1, "wait-node-is %d, prenode-is %d\n", node, prenode);
		while(node->locked == 1)
			;
		// printf(1, "wait-end-is %d\n", node->id);
	}

	// printf(1, "thread %d end-acquire\n", node->id);

}

int cmpxchg (int *__ptr, int __old, int __new){
// asm volatile("movl $0, %0" : "+m" (lk->locked) : );
	uint __ret;

	asm volatile("lock; cmpxchgl %2, %1"
		: "=a" (__ret), "+m" (*__ptr)
		: "r" (__new), "0" (__old)
		:"memory");

	return __ret;
}

void release_mcslock(struct mymcslock *lk, struct mymcsnode *node){
	// printf(1, "thread %d start release\n", node->id);

	struct mymcsnode *tmp = node;
	if(node->next == NULL){
		if(cmpxchg((int *)&lk->tail, (int)tmp, (int)NULL) == (int)node){
			// printf(1, "thread %d end release\n", node->id);
			return;
		}
		while (node->next == NULL)
			;
	}
	// printf(1, "release-node id is %d, next node is %d\n", node, node->next);
	// printf(1, "thread %d end release\n", node->id);
	node->next->locked = 0;
}

void start_routine(void *arg){

	int* threadid = (int *)arg;
	int next;
	if(*threadid < threads-1)
		next = *threadid + 1;
	else
		next = 0;

	struct mymcsnode mynode;
	mynode.next = NULL;
	mynode.id = *threadid;

	while(token.count < passes){
		// break
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


