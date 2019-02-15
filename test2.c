#include "types.h"
#include "user.h"

int main(int argc, char *argv[]){
	// int test1 (void);

	// test1();

	int tick = 20;
	settickets(tick);
	int i, k;

	const int loop=43000;
	for(i=0;i < loop;i++)
	{
		for(k=0;k<loop;k++){
			asm("nop");
		}
	}
	printf(1, "\n\nprocess with %d tickets finished !!\n", tick);
	exit();

	return 0;
}

// int test1 (void){

	// int pid;
	// int i;

	// for (i = 0; i < 3; i ++){
	// 	pid = fork();
	// 	if (pid > 0){
	// 		continue;
	// 	}
	// 	else if(pid == 0){
			
	// 	}
	// }


// }