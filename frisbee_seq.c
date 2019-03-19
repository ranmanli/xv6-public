#include "types.h"
#include "user.h"
#include "x86.h"

#define PAGESIZE      4096

struct myseqlock {
  uint locked;      
  volatile uint sequence;
};

struct frisbee {
	uint value;
	struct myseqlock lock;
	uint count;
};

uint threads;
uint passes;
struct frisbee token;


void init_myseqlock(struct myseqlock *lk){
	lk->locked = 0;
	lk->sequence = 0;
}

void acquire_seqlock_write(struct myseqlock *lk){
	lk->sequence++;
	while(xchg(&lk->locked, 1) != 0)
		;
	__sync_synchronize();
}

void release_seqlock_write(struct myseqlock *lk){
	__sync_synchronize();
	lk->sequence++;
	asm volatile("movl $0, %0" : "+m" (lk->locked) : );
}

uint acquire_seqlock_read(struct myseqlock *lk){
	while((lk->sequence%2) != 0)
		;
	
	return lk->sequence;
}

void start_routine(void *arg){
	int threadid = *(int *)arg;
	int next;
	uint END = 0;
	uint flag = 0;
	uint seqb;
	uint thisvalue, thiscount;
	if(threadid < threads-1)
		next = threadid + 1;
	else
		next = 0;

	do{

		do{
			seqb = acquire_seqlock_read(&token.lock);
			thiscount = token.count;
			thisvalue = token.value;
			if(thiscount > passes){
				END = 1;
				break;
			}
			if(thisvalue == threadid && thiscount <= passes){
				flag = 1;
			}
		}while(seqb != token.lock.sequence);

		if(flag){
			acquire_seqlock_write(&token.lock);
			token.value = next;
			printf(1, "Pass number no: %d, Thread %d is passing the token to thread %d\n",token.count, threadid, next);	
			token.count++;
			flag = 0;
			release_seqlock_write(&token.lock);
		}
		
	}while(!END);


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
	init_myseqlock(&token.lock);
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

	finish = uptime();
	printf(1, "running time : %d\n", (finish-start));
	exit();

	return 0;
}


