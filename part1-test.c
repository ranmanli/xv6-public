#include "types.h"
#include "user.h"

int main(int argc, char *argv){
	int test1 (void);

	test1();
	return 0;
}

int test1 (void){

	int pid;
	int i;

	for (i = 0; i < 3; i ++){
		pid = fork();
		if (pid > 0){
			continue;
		}
		else if(pid == 0){
			
		}
	}
}